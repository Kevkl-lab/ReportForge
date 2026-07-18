#include "pdf_generator.h"
#include <QPdfWriter>
#include <QPainter>
#include <QTextDocument>
#include <QDateTime>
#include <QDir>
#include <QFileInfo>
#include <QImage>
#include <QBuffer>
#include <QPainterPath>
#include <QDebug>
#include <map>
#include <cmath>

namespace reportforge::services {

// Premium Color Palettes (enterprise light content, deep dark cover)
static const QColor DarkBg(11, 11, 15);         // Deep space black cover background
static const QColor AccentPurple(139, 92, 246); // Royal neon violet primary (#8b5cf6)
static const QColor AccentBlue(6, 182, 212);    // Neon cyan secondary (#06b6d4)
static const QColor TextWhite(255, 255, 255);
static const QColor TextDark(30, 41, 59);       // Modern dark grey text (#1e293b)
static const QColor TextMuted(100, 116, 139);   // Cool slate grey for labels (#64748b)
static const QColor CodeBg(248, 250, 252);      // Slate light gray for code blocks (#f8fafc)
static const QColor BorderLight(226, 232, 240); // Cool gray border (#e2e8f0)

// Helper to get severity colors matching international standards
static QColor getSeverityColor(core::Severity severity) {
    switch (severity) {
        case core::Severity::Critical:      return QColor(153, 27, 27);  // Crimson Red (#991b1b)
        case core::Severity::High:          return QColor(220, 38, 38);  // Red (#dc2626)
        case core::Severity::Medium:        return QColor(217, 119, 6);  // Amber (#d97706)
        case core::Severity::Low:           return QColor(37, 99, 235);  // Royal Blue (#2563eb)
        case core::Severity::Informational: return QColor(71, 85, 105);   // Slate Grey (#475569)
    }
    return QColor(100, 116, 139);
}

// Helper to get light backgrounds for severity tags in tables
static QColor getSeverityBgColor(core::Severity severity) {
    switch (severity) {
        case core::Severity::Critical:      return QColor(254, 242, 242); // Light red
        case core::Severity::High:          return QColor(255, 245, 245);
        case core::Severity::Medium:        return QColor(254, 243, 199); // Light amber
        case core::Severity::Low:           return QColor(239, 246, 255); // Light blue
        case core::Severity::Informational: return QColor(241, 245, 249); // Light grey
    }
    return QColor(241, 245, 249);
}

// ----------------------------------------------------
// VECTOR ICON GENERATOR
// Renders vector icons natively via QPainterPath (DPI-scaled sizes)
// ----------------------------------------------------
static void drawShieldIcon(QPainter& painter, int x, int y, int size, double scale) {
    painter.save();
    painter.setRenderHint(QPainter::Antialiasing);
    painter.setPen(QPen(AccentPurple, std::max(1.0, 1.5 * scale)));
    painter.setBrush(QBrush(QColor(139, 92, 246, 20)));

    QPainterPath path;
    path.moveTo(x + size/2.0, y);
    path.lineTo(x + size, y + size/4.0);
    path.lineTo(x + size, y + size/2.0);
    path.quadTo(x + size, y + size*7.0/8.0, x + size/2.0, y + size);
    path.quadTo(x, y + size*7.0/8.0, x, y + size/2.0);
    path.lineTo(x, y + size/4.0);
    path.closeSubpath();
    painter.drawPath(path);

    painter.restore();
}

static void drawBugIcon(QPainter& painter, int x, int y, int size, double scale) {
    painter.save();
    painter.setRenderHint(QPainter::Antialiasing);
    painter.setPen(QPen(AccentPurple, std::max(1.0, 1.5 * scale)));
    painter.setBrush(Qt::NoBrush);

    // Body
    painter.drawEllipse(x + size/4, y + size/4, size/2, size/2);
    // Head
    painter.drawEllipse(x + size*3/8, y + size/8, size/4, size/4);

    // Legs
    painter.drawLine(x + size/4, y + size/2, x + size/10, y + size/2);
    painter.drawLine(x + size/4, y + size*3/8, x + size/10, y + size/4);
    painter.drawLine(x + size/4, y + size*5/8, x + size/10, y + size*3/4);

    painter.drawLine(x + size*3/4, y + size/2, x + size*9/10, y + size/2);
    painter.drawLine(x + size*3/4, y + size*3/8, x + size*9/10, y + size/4);
    painter.drawLine(x + size*3/4, y + size*5/8, x + size*9/10, y + size*3/4);

    // Antennae
    painter.drawLine(x + size/2, y + size/8, x + size*3/8, y);
    painter.drawLine(x + size/2, y + size/8, x + size*5/8, y);

    painter.restore();
}

static void drawTargetIcon(QPainter& painter, int x, int y, int size, double scale) {
    painter.save();
    painter.setRenderHint(QPainter::Antialiasing);
    painter.setPen(QPen(AccentBlue, std::max(1.0, 1.5 * scale)));
    painter.setBrush(Qt::NoBrush);
    
    painter.drawEllipse(x, y, size, size);
    painter.drawEllipse(x + size/4, y + size/4, size/2, size/2);
    
    painter.setBrush(AccentBlue);
    painter.drawEllipse(x + size*3/8, y + size*3/8, size/4, size/4);
    
    painter.drawLine(x - 3 * scale, y + size/2, x + size + 3 * scale, y + size/2);
    painter.drawLine(x + size/2, y - 3 * scale, x + size/2, y + size + 3 * scale);
    painter.restore();
}

static void drawCalendarIcon(QPainter& painter, int x, int y, int size, double scale) {
    painter.save();
    painter.setRenderHint(QPainter::Antialiasing);
    painter.setPen(QPen(AccentBlue, std::max(1.0, 1.5 * scale)));
    painter.setBrush(Qt::NoBrush);
    painter.drawRoundedRect(x, y + size/8, size, size*7/8, 2 * scale, 2 * scale);

    painter.drawLine(x, y + size/3, x + size, y + size/3);
    painter.drawLine(x + size/4, y, x + size/4, y + size/4);
    painter.drawLine(x + size*3/4, y, x + size*3/4, y + size/4);

    // Mini dots grid inside calendar
    painter.fillRect(x + size/4, y + size/2, 2 * scale, 2 * scale, AccentBlue);
    painter.fillRect(x + size/2, y + size/2, 2 * scale, 2 * scale, AccentBlue);
    painter.fillRect(x + size*3/4, y + size/2, 2 * scale, 2 * scale, AccentBlue);
    painter.fillRect(x + size/4, y + size*3/4, 2 * scale, 2 * scale, AccentBlue);
    painter.fillRect(x + size/2, y + size*3/4, 2 * scale, 2 * scale, AccentBlue);
    painter.fillRect(x + size*3/4, y + size*3/4, 2 * scale, 2 * scale, AccentBlue);

    painter.restore();
}

static void drawWarningIcon(QPainter& painter, int x, int y, int size, double scale) {
    painter.save();
    painter.setRenderHint(QPainter::Antialiasing);
    painter.setPen(QPen(QColor(217, 119, 6), std::max(1.0, 1.5 * scale))); // Amber
    painter.setBrush(Qt::NoBrush);

    QPainterPath path;
    path.moveTo(x + size/2.0, y);
    path.lineTo(x + size, y + size);
    path.lineTo(x, y + size);
    path.closeSubpath();
    painter.drawPath(path);

    // Draw exclamation mark
    painter.drawLine(x + size/2, y + size/3, x + size/2, y + size*2/3);
    painter.fillRect(x + size/2 - 1 * scale, y + size*3/4, 2 * scale, 2 * scale, QColor(217, 119, 6));

    painter.restore();
}

static void drawDocumentIcon(QPainter& painter, int x, int y, int size, double scale) {
    painter.save();
    painter.setRenderHint(QPainter::Antialiasing);
    painter.setPen(QPen(AccentPurple, std::max(1.0, 1.5 * scale)));
    painter.setBrush(Qt::NoBrush);

    QPainterPath path;
    path.moveTo(x, y);
    path.lineTo(x + size*3/4.0, y);
    path.lineTo(x + size, y + size/4.0);
    path.lineTo(x + size, y + size);
    path.lineTo(x, y + size);
    path.closeSubpath();
    painter.drawPath(path);

    painter.drawLine(x + size*3/4.0, y, x + size*3/4.0, y + size/4.0);
    painter.drawLine(x + size*3/4.0, y + size/4.0, x + size, y + size/4.0);

    // Line stubs
    painter.drawLine(x + size/4, y + size/3, x + size*3/4, y + size/3);
    painter.drawLine(x + size/4, y + size/2, x + size*3/4, y + size/2);
    painter.drawLine(x + size/4, y + size*2/3, x + size*3/4, y + size*2/3);

    painter.restore();
}

static void drawImageIcon(QPainter& painter, int x, int y, int size, double scale) {
    painter.save();
    painter.setRenderHint(QPainter::Antialiasing);
    painter.setPen(QPen(AccentBlue, std::max(1.0, 1.5 * scale)));
    painter.setBrush(Qt::NoBrush);
    painter.drawRect(x, y, size, size);

    // Mountains shape
    QPainterPath path;
    path.moveTo(x, y + size);
    path.lineTo(x + size/3.0, y + size/2.0);
    path.lineTo(x + size*2/3.0, y + size*4.0/5.0);
    path.lineTo(x + size, y + size/3.0);
    path.lineTo(x + size, y + size);
    path.closeSubpath();
    painter.drawPath(path);

    painter.drawEllipse(x + size*2/3, y + size/6, size/6, size/6);

    painter.restore();
}

// ----------------------------------------------------
// TWO-PASS LAYOUT CONTEXT
// ----------------------------------------------------
struct LayoutContext {
    bool dummyPass{true};
    int currentPage{1};
    int totalPages{1};
    int width{0};  // Page width in pixels
    int height{0}; // Page height in pixels
    int margin{0}; // Margin in pixels
    double scale{1.0}; // DPI scale factor
    std::vector<std::pair<QString, int>> tocEntries; // Dynamic section page mapping
};

// Global Page Header and Footer renderer (drawing in pixel coordinates)
static void drawHeaderFooter(QPainter& painter, const QString& projectTitle, const QString& currentSection, int pageNum, int totalPages, int width, int height, int margin, double scale) {
    painter.save();
    auto pt = [&](double pts) { return std::round(pts * scale); };
    
    // Header
    painter.setFont(QFont("Arial", 8, QFont::Normal));
    painter.setPen(TextMuted);
    painter.drawText(margin, margin - pt(12), QString("ReportForge Security Assessment | %1").arg(projectTitle));
    
    if (!currentSection.isEmpty()) {
        int secWidth = painter.fontMetrics().horizontalAdvance(currentSection);
        painter.drawText(width - margin - secWidth, margin - pt(12), currentSection);
    }
    
    painter.setPen(QPen(BorderLight, std::max(1.0, 1.0 * scale)));
    painter.drawLine(margin, margin - pt(6), width - margin, margin - pt(6));

    // Footer
    painter.drawLine(margin, height - margin + pt(6), width - margin, height - margin + pt(6));
    
    painter.setFont(QFont("Arial", 8, QFont::Normal));
    painter.setPen(TextMuted);
    painter.drawText(margin, height - margin + pt(18), "CLASSIFICATION: CONFIDENTIAL");
    
    QString pageStr = QString("Page %1 of %2").arg(pageNum).arg(totalPages);
    painter.drawText(width - margin - painter.fontMetrics().horizontalAdvance(pageStr), height - margin + pt(18), pageStr);

    painter.restore();
}

// Helper to render Markdown using QTextDocument (drawing in pixel coordinates, scaled from 96 to 300 DPI)
static void renderMarkdownBlock(QPainter& painter, QPdfWriter* device, LayoutContext& ctx, const QString& projectTitle, const QString& sectionName, const QString& markdown, int& y) {
    QTextDocument doc;
    auto pt = [&](double pts) { return std::round(pts * ctx.scale); };
    
    // Set custom stylesheet on the QTextDocument to style HTML tags
    doc.setDefaultStyleSheet(
        "body { font-family: Arial; font-size: 10pt; color: #23293b; line-height: 140%; }"
        "h1 { font-family: Arial; font-size: 13pt; color: #8b5cf6; margin-top: 12px; margin-bottom: 4px; font-weight: bold; }"
        "h2 { font-family: Arial; font-size: 10.5pt; color: #06b6d4; margin-top: 8px; margin-bottom: 4px; font-weight: bold; }"
        "code { font-family: 'Courier New', monospace; font-size: 9pt; background-color: #f8fafc; color: #0f172a; }"
        "ul { margin-top: 4px; margin-bottom: 4px; padding-left: 20px; }"
        "li { margin-bottom: 2px; }"
    );
    
    doc.setDefaultFont(QFont("Arial", 10));
    doc.setMarkdown(markdown);
    
    // Scale document layout from default 96 DPI to printer DPI
    double docScale = device->resolution() / 96.0;
    double docWidth = (ctx.width - 2 * ctx.margin) / docScale;
    doc.setTextWidth(docWidth);
    double docHeight = doc.size().height() * docScale;

    // Check pagination
    if (y + docHeight > ctx.height - ctx.margin) {
        ctx.currentPage++;
        if (!ctx.dummyPass) {
            device->newPage();
            drawHeaderFooter(painter, projectTitle, sectionName, ctx.currentPage, ctx.totalPages, ctx.width, ctx.height, ctx.margin, ctx.scale);
        }
        y = ctx.margin + pt(20);
    }

    if (!ctx.dummyPass) {
        painter.save();
        painter.translate(ctx.margin, y);
        painter.scale(docScale, docScale);
        doc.drawContents(&painter);
        painter.restore();
    }
    
    y += docHeight;
}

// ----------------------------------------------------
// TWO-PASS DOCUMENT LAYOUT RENDERER
// ----------------------------------------------------
static void renderDocument(QPainter& painter, QPdfWriter* device, LayoutContext& ctx, const models::Project& project, const std::vector<models::Finding>& findings, const std::vector<models::Evidence>& evidenceList) {
    auto pt = [&](double pts) { return std::round(pts * ctx.scale); };
    double docScale = device->resolution() / 96.0;
    double docWidth = (ctx.width - 2 * ctx.margin) / docScale;

    QString projectTitle = QString::fromStdString(project.name);
    
    QFont coverTitleFont("Arial", 28, QFont::Bold);
    QFont coverMetaFont("Arial", 10, QFont::Normal);
    QFont h1Font("Arial", 16, QFont::Bold);
    QFont h2Font("Arial", 11, QFont::Bold);
    QFont bodyFont("Arial", 10, QFont::Normal);
    QFont monoFont("Courier New", 9, QFont::Normal);

    // ----------------------------------------------------
    // PAGE 1: PREMIUM COVER PAGE (Dark Branding)
    // ----------------------------------------------------
    if (!ctx.dummyPass) {
        painter.save();
        // Solid dark background
        painter.fillRect(0, 0, ctx.width, ctx.height, DarkBg);

        // Grid lines to form a clean security tech feel
        painter.setPen(QPen(QColor(139, 92, 246, 30), std::max(1.0, 0.75 * ctx.scale)));
        for (int i = ctx.width / 10; i < ctx.width; i += ctx.width / 10) {
            painter.drawLine(i, 0, i, ctx.height);
        }
        for (int i = ctx.height / 15; i < ctx.height; i += ctx.height / 15) {
            painter.drawLine(0, i, ctx.width, i);
        }

        // Geometric banner lines
        painter.setPen(QPen(AccentPurple, pt(4)));
        painter.drawLine(ctx.margin, pt(180), ctx.width - ctx.margin, pt(180));
        painter.setPen(QPen(AccentBlue, pt(2)));
        painter.drawLine(ctx.margin, pt(188), ctx.width * 0.55, pt(188));

        // Document title (with newline, properly spaced)
        painter.setPen(TextWhite);
        painter.setFont(coverTitleFont);
        painter.drawText(ctx.margin, pt(270), "SECURITY ASSESSMENT\nREPORT");

        // Logo drawing
        painter.setFont(QFont("Arial", 14, QFont::Bold));
        painter.setPen(AccentPurple);
        painter.drawText(ctx.margin, pt(380), "REPORTFORGE");

        // Divider
        painter.setPen(QPen(BorderLight, std::max(1.0, 0.75 * ctx.scale)));
        painter.drawLine(ctx.margin, pt(420), ctx.width - ctx.margin, pt(420));

        // Metadata grid (Y spacing increased to 45 points, and label-to-value offset to 16 to eliminate overlaps)
        int metaY = pt(470);
        int col1X = ctx.margin;
        int col2X = ctx.width * 0.50; // Perfectly centered column 2 split

        auto drawMetaItem = [&](int x, int y, const QString& label, const QString& value) {
            painter.setFont(QFont("Arial", 8, QFont::Bold));
            painter.setPen(AccentBlue);
            painter.drawText(x, y, label.toUpper());
            painter.setFont(QFont("Arial", 10, QFont::Normal));
            painter.setPen(TextWhite);
            painter.drawText(x, y + pt(16), value);
        };

        drawMetaItem(col1X, metaY, "Target Customer", QString::fromStdString(project.customer));
        drawMetaItem(col2X, metaY, "Engagement Project", QString::fromStdString(project.name));

        drawMetaItem(col1X, metaY + pt(45), "Start Date", QString::fromStdString(project.startDate));
        drawMetaItem(col2X, metaY + pt(45), "End Date", QString::fromStdString(project.endDate));

        drawMetaItem(col1X, metaY + pt(90), "Status", QString::fromStdString(core::projectStatusToString(project.status)).toUpper());
        drawMetaItem(col2X, metaY + pt(90), "Classification", "CONFIDENTIAL");

        // Footer block
        painter.setFont(QFont("Arial", 8, QFont::Normal));
        painter.setPen(TextMuted);
        painter.drawText(ctx.margin, ctx.height - ctx.margin + pt(15), "Generated dynamically. Confidential deliverable. Property of respective client.");
        painter.restore();
    }

    // ----------------------------------------------------
    // PAGE 2: TABLE OF CONTENTS (Toc)
    // ----------------------------------------------------
    ctx.currentPage = 2;
    if (!ctx.dummyPass) {
        device->newPage();
        drawHeaderFooter(painter, projectTitle, "Table of Contents", ctx.currentPage, ctx.totalPages, ctx.width, ctx.height, ctx.margin, ctx.scale);

        int y = ctx.margin + pt(40);
        painter.setFont(h1Font);
        painter.setPen(AccentPurple);
        drawDocumentIcon(painter, ctx.margin, y - pt(4), pt(18), ctx.scale);
        painter.drawText(ctx.margin + pt(25), y + pt(13), "Table of Contents");
        y += pt(50);

        painter.setFont(bodyFont);
        painter.setPen(TextDark);

        auto drawTocLine = [&](const QString& title, int page) {
            QString pageStr = QString::number(page);
            int titleWidth = painter.fontMetrics().horizontalAdvance(title);
            int pageLen = painter.fontMetrics().horizontalAdvance(pageStr);
            int lineW = ctx.width - 2 * ctx.margin - titleWidth - pageLen - pt(25);

            painter.drawText(ctx.margin, y, title);
            
            // Draw dotted line (tab leaders)
            QString dots = "";
            int dotWidth = painter.fontMetrics().horizontalAdvance(".");
            int dotCount = lineW / dotWidth;
            for (int k = 0; k < dotCount; ++k) dots += ".";
            
            painter.setPen(TextMuted);
            painter.drawText(ctx.margin + titleWidth + pt(10), y, dots);
            
            painter.setPen(TextDark);
            painter.drawText(ctx.width - ctx.margin - pageLen, y, pageStr);
            y += pt(25);
        };

        for (const auto& entry : ctx.tocEntries) {
            drawTocLine(entry.first, entry.second);
        }
    }

    // ----------------------------------------------------
    // PAGE 3: EXECUTIVE SUMMARY & RISK CARDS
    // ----------------------------------------------------
    ctx.currentPage = 3;
    if (ctx.dummyPass) {
        ctx.tocEntries.push_back({"Executive Summary", ctx.currentPage});
    } else {
        device->newPage();
        drawHeaderFooter(painter, projectTitle, "Executive Summary", ctx.currentPage, ctx.totalPages, ctx.width, ctx.height, ctx.margin, ctx.scale);
    }

    int y = ctx.margin + pt(40);
    if (!ctx.dummyPass) {
        painter.setFont(h1Font);
        painter.setPen(AccentPurple);
        drawShieldIcon(painter, ctx.margin, y - pt(4), pt(18), ctx.scale);
        painter.drawText(ctx.margin + pt(25), y + pt(13), "1. Executive Summary");
    }
    y += pt(50);

    QString summaryText = QString::fromStdString(project.description);
    if (summaryText.isEmpty()) {
        summaryText = "No project description provided. This assessment was initiated to evaluate the overall security posture and identify exploitable vulnerabilities in the target system.";
    }
    renderMarkdownBlock(painter, device, ctx, projectTitle, "Executive Summary", summaryText, y);
    y += pt(30);

    // Title: Risk Profile Cards
    if (!ctx.dummyPass) {
        painter.setFont(h2Font);
        painter.setPen(AccentPurple);
        painter.drawText(ctx.margin, y, "Vulnerability Distribution Grid");
    }
    y += pt(20);

    // Count findings
    int crit = 0, high = 0, med = 0, low = 0, info = 0;
    for (const auto& f : findings) {
        switch (f.severity) {
            case core::Severity::Critical: crit++; break;
            case core::Severity::High: high++; break;
            case core::Severity::Medium: med++; break;
            case core::Severity::Low: low++; break;
            case core::Severity::Informational: info++; break;
        }
    }

    // Draw Grid Cards for Severities (DPI Scaled in pixels)
    int gridW = ctx.width - 2 * ctx.margin;
    int cardCount = 5;
    int spacing = pt(10);
    int cardW = (gridW - (cardCount - 1) * spacing) / cardCount;
    int cardH = pt(65); // Balanced card height in pixels

    std::vector<std::pair<QString, int>> cards = {
        {"CRITICAL", crit},
        {"HIGH", high},
        {"MEDIUM", med},
        {"LOW", low},
        {"INFO", info}
    };
    std::vector<core::Severity> cardEnums = {
        core::Severity::Critical,
        core::Severity::High,
        core::Severity::Medium,
        core::Severity::Low,
        core::Severity::Informational
    };

    if (!ctx.dummyPass) {
        painter.save();
        for (int i = 0; i < cardCount; ++i) {
            int cx = ctx.margin + i * (cardW + spacing);
            
            // Draw background card shadow/border
            QColor sColor = getSeverityColor(cardEnums[i]);
            painter.setPen(QPen(sColor, std::max(1.0, 1.5 * ctx.scale)));
            painter.setBrush(QBrush(getSeverityBgColor(cardEnums[i])));
            painter.drawRoundedRect(cx, y, cardW, cardH, pt(4), pt(4));

            // Title (centered at top)
            painter.setFont(QFont("Arial", 7, QFont::Bold));
            painter.setPen(sColor);
            painter.drawText(QRect(cx, y + pt(10), cardW, pt(15)), Qt::AlignCenter, cards[i].first);

            // Number value (generous height, centered, no clipping)
            painter.setFont(QFont("Arial", 18, QFont::Bold));
            painter.setPen(sColor);
            painter.drawText(QRect(cx, y + pt(28), cardW, pt(28)), Qt::AlignCenter, QString::number(cards[i].second));
        }
        painter.restore();
    }
    y += cardH + pt(35);

    // Methodology Section
    if (!ctx.dummyPass) {
        painter.setFont(h2Font);
        painter.setPen(AccentPurple);
        painter.drawText(ctx.margin, y, "Methodology & Scope");
    }
    y += pt(16);

    QString methodologyText = 
        "Our assessment followed standard penetration testing lifecycle workflows, aligned with "
        "the Open Web Application Security Project (OWASP) Testing Framework and industry guidelines:\n\n"
        "- **Planning**: Reviewing engagement windows and testing parameters.\n"
        "- **Reconnaissance**: Gathering passive DNS data, OSINT intelligence, and mapping targets.\n"
        "- **Scanning**: Mapping active services, identifying vulnerabilities, and checking configs.\n"
        "- **Exploitation**: Demonstrating risk by safely executing payloads.\n"
        "- **Reporting**: Documenting findings, CVSS ratings, and remediation recommendations.";
    renderMarkdownBlock(painter, device, ctx, projectTitle, "Executive Summary", methodologyText, y);

    // ----------------------------------------------------
    // PAGE 4: SCOPE, TARGETS & ASSESSMENT TIMELINE
    // ----------------------------------------------------
    ctx.currentPage = 4;
    if (ctx.dummyPass) {
        ctx.tocEntries.push_back({"Scope & Timeline", ctx.currentPage});
    } else {
        device->newPage();
        drawHeaderFooter(painter, projectTitle, "Scope & Timeline", ctx.currentPage, ctx.totalPages, ctx.width, ctx.height, ctx.margin, ctx.scale);
    }

    y = ctx.margin + pt(40);
    if (!ctx.dummyPass) {
        painter.setFont(h1Font);
        painter.setPen(AccentPurple);
        drawTargetIcon(painter, ctx.margin, y - pt(5), pt(20), ctx.scale);
        painter.drawText(ctx.margin + pt(25), y + pt(13), "2. Scope & Timeline");
    }
    y += pt(50);

    QString scopeTargets = "";
    if (!project.targets.empty()) {
        scopeTargets += "**Targets:**\n\n" + QString::fromStdString(project.targets) + "\n\n";
    }
    if (!project.scope.empty()) {
        scopeTargets += "**Scope Constraints:**\n\n" + QString::fromStdString(project.scope);
    }
    if (scopeTargets.isEmpty()) {
        scopeTargets = "No specific targets or scope constraints defined for this project.";
    }

    renderMarkdownBlock(painter, device, ctx, projectTitle, "Scope & Timeline", scopeTargets, y);
    y += pt(40);

    // Timeline Graphic Section
    if (!ctx.dummyPass) {
        painter.setFont(h2Font);
        painter.setPen(AccentPurple);
        drawCalendarIcon(painter, ctx.margin, y - pt(5), pt(20), ctx.scale);
        painter.drawText(ctx.margin + pt(25), y + pt(13), "Assessment Timeline");
    }
    y += pt(40);

    // Draw timeline (DPI Scaled in pixels)
    if (!ctx.dummyPass) {
        painter.save();
        painter.setRenderHint(QPainter::Antialiasing);

        int timelineW = ctx.width - 2 * ctx.margin - pt(80);
        int tY = y + pt(25);
        
        painter.setPen(QPen(BorderLight, std::max(1.0, 2.0 * ctx.scale)));
        painter.drawLine(ctx.margin + pt(40), tY, ctx.margin + pt(40) + timelineW, tY);

        std::vector<QString> steps = {"Planning", "Recon", "Scanning", "Exploitation", "Reporting"};
        int stepSpacing = timelineW / (steps.size() - 1);

        for (size_t i = 0; i < steps.size(); ++i) {
            int cx = ctx.margin + pt(40) + i * stepSpacing;
            
            // Draw dot
            painter.setPen(QPen(AccentPurple, std::max(1.0, 2.0 * ctx.scale)));
            painter.setBrush(AccentBlue);
            painter.drawEllipse(cx - pt(5), tY - pt(5), pt(10), pt(10));

            // Step Label (generous bounding box, no truncation)
            painter.setFont(QFont("Arial", 7, QFont::Bold));
            painter.setPen(TextDark);
            int labelW = pt(80);
            painter.drawText(QRect(cx - labelW/2, tY + pt(12), labelW, pt(25)), Qt::AlignCenter, steps[i]);
        }
        painter.restore();
    }
    y += pt(70);

    // ----------------------------------------------------
    // PAGE 5+: DETAILED FINDINGS LOG (Visual Redesign)
    // ----------------------------------------------------
    int findingCounter = 0;
    for (const auto& f : findings) {
        findingCounter++;
        ctx.currentPage++;
        
        QString findingSectionTitle = QString("Finding %1: %2").arg(findingCounter).arg(QString::fromStdString(f.title));
        
        if (ctx.dummyPass) {
            ctx.tocEntries.push_back({findingSectionTitle, ctx.currentPage});
        } else {
            device->newPage();
            drawHeaderFooter(painter, projectTitle, "Detailed Findings", ctx.currentPage, ctx.totalPages, ctx.width, ctx.height, ctx.margin, ctx.scale);
        }

        y = ctx.margin + pt(40);

        // Render Finding header with colored severity stripe (pixel coordinates)
        if (!ctx.dummyPass) {
            painter.save();
            painter.fillRect(ctx.margin, y, ctx.width - 2 * ctx.margin, pt(25), getSeverityColor(f.severity));
            
            painter.setFont(QFont("Arial", 10, QFont::Bold));
            painter.setPen(TextWhite);
            QString sevLabel = QString("  [%1] %2").arg(QString::fromStdString(core::severityToString(f.severity)).toUpper(), findingSectionTitle);
            painter.drawText(QRect(ctx.margin, y, ctx.width - 2 * ctx.margin, pt(25)), Qt::AlignVCenter | Qt::AlignLeft, sevLabel);
            painter.restore();
        }
        y += pt(45);

        // Metadata table (correct stylesheet resolution, escaped %% in HTML)
        QString htmlTable = QString(
            "<table>"
            "  <tr>"
            "    <th style='width: 30%%;'>Attribute</th>"
            "    <th style='width: 70%%;'>Detail Value</th>"
            "  </tr>"
            "  <tr>"
            "    <td style='font-weight: bold;'>Severity & Rating</td>"
            "    <td style='color: %1; font-weight: bold;'>%2 (CVSS: %3)</td>"
            "  </tr>"
            "  <tr>"
            "    <td style='font-weight: bold;'>Classification</td>"
            "    <td>%4 | %5</td>"
            "  </tr>"
            "  <tr>"
            "    <td style='font-weight: bold;'>Lifecycle Status</td>"
            "    <td>%6</td>"
            "  </tr>"
            "  <tr>"
            "    <td style='font-weight: bold;'>Affected Targets</td>"
            "    <td style='font-family: monospace;'>%7</td>"
            "  </tr>"
            "</table>"
        ).arg(getSeverityColor(f.severity).name())
         .arg(QString::fromStdString(core::severityToString(f.severity)))
         .arg(f.cvssScore, 0, 'f', 1)
         .arg(QString::fromStdString(f.cwe))
         .arg(QString::fromStdString(f.owaspCategory))
         .arg(QString::fromStdString(core::findingStatusToString(f.status)))
         .arg(QString::fromStdString(f.affectedAssets).toHtmlEscaped());

        QTextDocument tableDoc;
        tableDoc.setTextWidth(docWidth);
        tableDoc.setDefaultFont(QFont("Arial", 9));
        tableDoc.setDefaultStyleSheet(
            "body { font-family: Arial; font-size: 9.5pt; color: #1e293b; }"
            "table { width: 100%; border-collapse: collapse; border: 1px solid #cbd5e1; }"
            "th { background-color: #f8fafc; font-weight: bold; padding: 6px; border: 1px solid #cbd5e1; color: #475569; }"
            "td { padding: 6px; border: 1px solid #cbd5e1; }"
        );
        tableDoc.setHtml(htmlTable);
        double tableH = tableDoc.size().height() * docScale;

        if (!ctx.dummyPass) {
            painter.save();
            painter.translate(ctx.margin, y);
            painter.scale(docScale, docScale);
            tableDoc.drawContents(&painter);
            painter.restore();
        }
        y += tableH + pt(25);

        // Subsections: Description, Impact, Recommendation
        auto renderSubsection = [&](const QString& name, const QString& mdContent, const QString& iconType) {
            if (mdContent.isEmpty()) return;

            if (y + pt(80) > ctx.height - ctx.margin) {
                ctx.currentPage++;
                if (!ctx.dummyPass) {
                    device->newPage();
                    drawHeaderFooter(painter, projectTitle, "Detailed Findings", ctx.currentPage, ctx.totalPages, ctx.width, ctx.height, ctx.margin, ctx.scale);
                }
                y = ctx.margin + pt(20);
            }

            if (!ctx.dummyPass) {
                painter.setFont(h2Font);
                painter.setPen(AccentPurple);
                if (iconType == "bug") drawBugIcon(painter, ctx.margin, y - pt(5), pt(16), ctx.scale);
                else if (iconType == "warning") drawWarningIcon(painter, ctx.margin, y - pt(5), pt(16), ctx.scale);
                else if (iconType == "shield") drawShieldIcon(painter, ctx.margin, y - pt(5), pt(16), ctx.scale);
                
                painter.drawText(ctx.margin + pt(22), y + pt(11), name);
            }
            y += pt(20);

            renderMarkdownBlock(painter, device, ctx, projectTitle, "Detailed Findings", mdContent, y);
            y += pt(15);
        };

        renderSubsection("Vulnerability Description", QString::fromStdString(f.description), "bug");
        renderSubsection("Risk & Exploit Impact", QString::fromStdString(f.impact), "warning");
        renderSubsection("Remediation Recommendation", QString::fromStdString(f.recommendation), "shield");

        // Code block for Proof of Concept (Monospace card with grey background)
        if (!f.proofOfConcept.empty()) {
            if (y + pt(70) > ctx.height - ctx.margin) {
                ctx.currentPage++;
                if (!ctx.dummyPass) {
                    device->newPage();
                    drawHeaderFooter(painter, projectTitle, "Detailed Findings", ctx.currentPage, ctx.totalPages, ctx.width, ctx.height, ctx.margin, ctx.scale);
                }
                y = ctx.margin + pt(20);
            }

            if (!ctx.dummyPass) {
                painter.setFont(h2Font);
                painter.setPen(AccentPurple);
                drawDocumentIcon(painter, ctx.margin, y - pt(5), pt(16), ctx.scale);
                painter.drawText(ctx.margin + pt(22), y + pt(11), "Proof of Concept / Technical Payload");
            }
            y += pt(20);

            // Formulate Code Block text layout (in pixels, scaled)
            QTextDocument codeDoc;
            codeDoc.setDefaultFont(monoFont);
            codeDoc.setTextWidth((ctx.width - 2 * ctx.margin - pt(30)) / docScale);
            codeDoc.setPlainText(QString::fromStdString(f.proofOfConcept));
            double codeH = codeDoc.size().height() * docScale;

            if (y + codeH + pt(20) > ctx.height - ctx.margin) {
                ctx.currentPage++;
                if (!ctx.dummyPass) {
                    device->newPage();
                    drawHeaderFooter(painter, projectTitle, "Detailed Findings", ctx.currentPage, ctx.totalPages, ctx.width, ctx.height, ctx.margin, ctx.scale);
                }
                y = ctx.margin + pt(20);
            }

            if (!ctx.dummyPass) {
                painter.save();
                painter.setPen(QPen(BorderLight, std::max(1.0, 1.0 * ctx.scale)));
                painter.setBrush(QBrush(CodeBg));
                painter.drawRoundedRect(ctx.margin, y, ctx.width - 2 * ctx.margin, codeH + pt(15), pt(4), pt(4));
                
                painter.translate(ctx.margin + pt(15), y + pt(8));
                painter.scale(docScale, docScale);
                codeDoc.drawContents(&painter);
                painter.restore();
            }
            y += codeH + pt(30);
        }

        // Screenshots and Images embedding
        std::vector<models::Evidence> fEv;
        for (const auto& ev : evidenceList) {
            if (ev.findingId == f.id && ev.fileType == core::MediaType::Image) {
                fEv.push_back(ev);
            }
        }

        if (!fEv.empty()) {
            if (y + pt(50) > ctx.height - ctx.margin) {
                ctx.currentPage++;
                if (!ctx.dummyPass) {
                    device->newPage();
                    drawHeaderFooter(painter, projectTitle, "Detailed Findings", ctx.currentPage, ctx.totalPages, ctx.width, ctx.height, ctx.margin, ctx.scale);
                }
                y = ctx.margin + pt(20);
            }

            if (!ctx.dummyPass) {
                painter.setFont(h2Font);
                painter.setPen(AccentPurple);
                drawImageIcon(painter, ctx.margin, y - pt(5), pt(16), ctx.scale);
                painter.drawText(ctx.margin + pt(22), y + pt(11), "Vulnerability Evidence / Screenshots");
            }
            y += pt(25);
        }

        int figCounter = 0;
        for (const auto& ev : fEv) {
            QFileInfo fi(QString::fromStdString(ev.filePath));
            if (!fi.exists()) continue;

            QImage img(fi.absoluteFilePath());
            if (img.isNull()) continue;

            figCounter++;

            // Max dimensions inside margins
            int maxW = ctx.width - 2 * ctx.margin;
            int targetW = maxW;
            int targetH = img.height() * targetW / img.width();

            // Check page boundaries
            if (y + targetH + pt(40) > ctx.height - ctx.margin) {
                ctx.currentPage++;
                if (!ctx.dummyPass) {
                    device->newPage();
                    drawHeaderFooter(painter, projectTitle, "Detailed Findings", ctx.currentPage, ctx.totalPages, ctx.width, ctx.height, ctx.margin, ctx.scale);
                }
                y = ctx.margin + pt(20);
            }

            if (!ctx.dummyPass) {
                painter.save();
                painter.setRenderHint(QPainter::Antialiasing);
                
                // Draw centered image (scaled to target pixels)
                int imgX = ctx.margin + (maxW - targetW) / 2;
                painter.drawImage(QRect(imgX, y, targetW, targetH), img);

                // Draw thin picture outline border
                painter.setPen(QPen(BorderLight, std::max(1.0, 1.0 * ctx.scale)));
                painter.setBrush(Qt::NoBrush);
                painter.drawRect(imgX, y, targetW, targetH);
                
                // Draw caption
                painter.setFont(QFont("Arial", 8, QFont::StyleItalic));
                painter.setPen(TextMuted);
                QString caption = QString("Figure %1 - %2").arg(figCounter).arg(QString::fromStdString(ev.fileName));
                
                int capW = painter.fontMetrics().horizontalAdvance(caption);
                painter.drawText(ctx.margin + (maxW - capW) / 2, y + targetH + pt(15), caption);
                painter.restore();
            }
            y += targetH + pt(30);
        }
    }

    // ----------------------------------------------------
    // PAGE FINAL: APPENDIX & REFERENCE TABLES
    // ----------------------------------------------------
    ctx.currentPage++;
    if (ctx.dummyPass) {
        ctx.tocEntries.push_back({"Appendix", ctx.currentPage});
    } else {
        device->newPage();
        drawHeaderFooter(painter, projectTitle, "Appendix", ctx.currentPage, ctx.totalPages, ctx.width, ctx.height, ctx.margin, ctx.scale);
    }

    y = ctx.margin + pt(40);
    if (!ctx.dummyPass) {
        painter.setFont(h1Font);
        painter.setPen(AccentPurple);
        drawDocumentIcon(painter, ctx.margin, y - pt(5), pt(20), ctx.scale);
        painter.drawText(ctx.margin + pt(30), y + pt(14), "Appendix: Threat Classifications");
    }
    y += pt(50);

    QString appendixTable = 
        "<table>"
        "  <tr>"
        "    <th style='width: 25%%;'>Rating</th>"
        "    <th style='width: 20%%;'>CVSS Score</th>"
        "    <th style='width: 55%%;'>Threat Description</th>"
        "  </tr>"
        "  <tr>"
        "    <td style='color: #991b1b; font-weight: bold;'>CRITICAL</td>"
        "    <td style='font-weight: bold;'>9.0 - 10.0</td>"
        "    <td>Exploitation leads to remote shell access, full domain takeover, or database exfiltration.</td>"
        "  </tr>"
        "  <tr>"
        "    <td style='color: #dc2626; font-weight: bold;'>HIGH</td>"
        "    <td style='font-weight: bold;'>7.0 - 8.9</td>"
        "    <td>Exploitation allows sensitive data reading, local user takeover, or access bypass.</td>"
        "  </tr>"
        "  <tr>"
        "    <td style='color: #d97706; font-weight: bold;'>MEDIUM</td>"
        "    <td style='font-weight: bold;'>4.0 - 6.9</td>"
        "    <td>Vulnerability requires user interaction (e.g. CSRF, XSS) or specific configs.</td>"
        "  </tr>"
        "  <tr>"
        "    <td style='color: #2563eb; font-weight: bold;'>LOW</td>"
        "    <td style='font-weight: bold;'>0.1 - 3.9</td>"
        "    <td>Hardening details or information disclosures. Minimal threat profile.</td>"
        "  </tr>"
        "</table>";

    QTextDocument appTableDoc;
    appTableDoc.setTextWidth(docWidth);
    appTableDoc.setDefaultFont(QFont("Arial", 9));
    appTableDoc.setDefaultStyleSheet(
        "body { font-family: Arial; font-size: 9pt; color: #1e293b; }"
        "table { width: 100%; border-collapse: collapse; border: 1px solid #cbd5e1; }"
        "th { background-color: #f8fafc; font-weight: bold; padding: 6px; border: 1px solid #cbd5e1; color: #475569; }"
        "td { padding: 6px; border: 1px solid #cbd5e1; }"
    );
    appTableDoc.setHtml(appendixTable);
    double appTableH = appTableDoc.size().height() * docScale;

    if (!ctx.dummyPass) {
        painter.save();
        painter.translate(ctx.margin, y);
        painter.scale(docScale, docScale);
        appTableDoc.drawContents(&painter);
        painter.restore();
    }
    y += appTableH;
}

// ----------------------------------------------------
// PDF GENERATION PUBLIC APIs
// ----------------------------------------------------
bool PdfGenerator::generateReport(const std::string& outputPath,
                               const models::Project& project,
                               const std::vector<models::Finding>& findings,
                               const std::vector<models::Evidence>& evidenceList) {
    // PASS 1: GATHER PAGE METRICS ON A BUFFER DEVICE (0 page margins)
    QByteArray bufferBytes;
    QBuffer bufferDevice(&bufferBytes);
    bufferDevice.open(QIODevice::WriteOnly);
    
    QPdfWriter dummyWriter(&bufferDevice);
    dummyWriter.setPageSize(QPageSize(QPageSize::A4));
    dummyWriter.setResolution(300);
    dummyWriter.setPageMargins(QMarginsF(0, 0, 0, 0), QPageLayout::Point);
    
    LayoutContext ctx;
    ctx.dummyPass = true;
    ctx.currentPage = 1;
    ctx.scale = dummyWriter.resolution() / 72.0;
    ctx.width = dummyWriter.width();
    ctx.height = dummyWriter.height();
    ctx.margin = std::round(54.0 * ctx.scale); // 0.75 inch margin

    {
        QPainter dummyPainter(&dummyWriter);
        renderDocument(dummyPainter, &dummyWriter, ctx, project, findings, evidenceList);
    }
    
    int finalTotalPages = ctx.currentPage;
    auto tocMap = ctx.tocEntries;
    bufferDevice.close();

    // PASS 2: RENDER TRUE PRINT DOCUMENT TO THE ACTUAL TARGET FILE PATH (0 page margins)
    QPdfWriter realWriter(QString::fromStdString(outputPath));
    realWriter.setPageSize(QPageSize(QPageSize::A4));
    realWriter.setResolution(300);
    realWriter.setPageMargins(QMarginsF(0, 0, 0, 0), QPageLayout::Point);

    LayoutContext realCtx;
    realCtx.dummyPass = false;
    realCtx.currentPage = 1;
    realCtx.totalPages = finalTotalPages;
    realCtx.tocEntries = tocMap;
    realCtx.scale = realWriter.resolution() / 72.0;
    realCtx.width = realWriter.width();
    realCtx.height = realWriter.height();
    realCtx.margin = std::round(54.0 * realCtx.scale);

    QPainter realPainter(&realWriter);
    renderDocument(realPainter, &realWriter, realCtx, project, findings, evidenceList);

    return true;
}

std::vector<std::string> PdfGenerator::validateFindings(const std::vector<models::Finding>& findings,
                                                    const std::vector<models::Evidence>& evidenceList) {
    std::vector<std::string> warnings;

    // Cache screenshot count per finding
    std::map<int, int> screenshotCounts;
    for (const auto& ev : evidenceList) {
        if (ev.findingId > 0 && ev.fileType == core::MediaType::Image) {
            screenshotCounts[ev.findingId]++;
        }
    }

    for (const auto& f : findings) {
        std::vector<std::string> missing;
        if (f.description.empty() || f.description == "") {
            missing.push_back("Description");
        }
        if (f.impact.empty() || f.impact == "") {
            missing.push_back("Risk Impact");
        }
        if (f.recommendation.empty() || f.recommendation == "") {
            missing.push_back("Remediation Recommendation");
        }
        if (f.proofOfConcept.empty() || f.proofOfConcept == "") {
            missing.push_back("Proof of Concept");
        }
        if (screenshotCounts[f.id] == 0) {
            missing.push_back("Screenshot Evidence");
        }

        if (!missing.empty()) {
            std::string issue = "Finding '" + f.title + "' has incomplete fields: ";
            for (size_t i = 0; i < missing.size(); ++i) {
                issue += missing[i] + (i == missing.size() - 1 ? "" : ", ");
            }
            warnings.push_back(issue);
        }
    }

    return warnings;
}

} // namespace reportforge::services

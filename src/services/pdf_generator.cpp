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
// Renders vector icons natively via QPainterPath
// ----------------------------------------------------
static void drawShieldIcon(QPainter& painter, int x, int y, int size) {
    painter.save();
    painter.setRenderHint(QPainter::Antialiasing);
    painter.setPen(QPen(AccentPurple, 2));
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

static void drawBugIcon(QPainter& painter, int x, int y, int size) {
    painter.save();
    painter.setRenderHint(QPainter::Antialiasing);
    painter.setPen(QPen(AccentPurple, 2));
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

static void drawTargetIcon(QPainter& painter, int x, int y, int size) {
    painter.save();
    painter.setRenderHint(QPainter::Antialiasing);
    painter.setPen(QPen(AccentBlue, 2));
    painter.setBrush(Qt::NoBrush);
    
    painter.drawEllipse(x, y, size, size);
    painter.drawEllipse(x + size/4, y + size/4, size/2, size/2);
    
    painter.setBrush(AccentBlue);
    painter.drawEllipse(x + size*3/8, y + size*3/8, size/4, size/4);
    
    painter.drawLine(x - 5, y + size/2, x + size + 5, y + size/2);
    painter.drawLine(x + size/2, y - 5, x + size/2, y + size + 5);
    painter.restore();
}

static void drawCalendarIcon(QPainter& painter, int x, int y, int size) {
    painter.save();
    painter.setRenderHint(QPainter::Antialiasing);
    painter.setPen(QPen(AccentBlue, 2));
    painter.setBrush(Qt::NoBrush);
    painter.drawRoundedRect(x, y + size/8, size, size*7/8, 4, 4);

    painter.drawLine(x, y + size/3, x + size, y + size/3);
    painter.drawLine(x + size/4, y, x + size/4, y + size/4);
    painter.drawLine(x + size*3/4, y, x + size*3/4, y + size/4);

    // Mini dots grid inside calendar
    painter.fillRect(x + size/4, y + size/2, 4, 4, AccentBlue);
    painter.fillRect(x + size/2, y + size/2, 4, 4, AccentBlue);
    painter.fillRect(x + size*3/4, y + size/2, 4, 4, AccentBlue);
    painter.fillRect(x + size/4, y + size*3/4, 4, 4, AccentBlue);
    painter.fillRect(x + size/2, y + size*3/4, 4, 4, AccentBlue);
    painter.fillRect(x + size*3/4, y + size*3/4, 4, 4, AccentBlue);

    painter.restore();
}

static void drawWarningIcon(QPainter& painter, int x, int y, int size) {
    painter.save();
    painter.setRenderHint(QPainter::Antialiasing);
    painter.setPen(QPen(QColor(217, 119, 6), 2)); // Amber
    painter.setBrush(Qt::NoBrush);

    QPainterPath path;
    path.moveTo(x + size/2.0, y);
    path.lineTo(x + size, y + size);
    path.lineTo(x, y + size);
    path.closeSubpath();
    painter.drawPath(path);

    // Draw exclamation mark
    painter.drawLine(x + size/2, y + size/3, x + size/2, y + size*2/3);
    painter.fillRect(x + size/2 - 1, y + size*3/4, 2, 2, QColor(217, 119, 6));

    painter.restore();
}

static void drawDocumentIcon(QPainter& painter, int x, int y, int size) {
    painter.save();
    painter.setRenderHint(QPainter::Antialiasing);
    painter.setPen(QPen(AccentPurple, 2));
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

static void drawImageIcon(QPainter& painter, int x, int y, int size) {
    painter.save();
    painter.setRenderHint(QPainter::Antialiasing);
    painter.setPen(QPen(AccentBlue, 2));
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
    int width{0};
    int height{0};
    int margin{0};
    std::vector<std::pair<QString, int>> tocEntries; // Dynamic section page mapping
};

// Global Page Header and Footer renderer
static void drawHeaderFooter(QPainter& painter, const QString& projectTitle, const QString& currentSection, int pageNum, int totalPages, int width, int height, int margin) {
    painter.save();
    
    // Header
    painter.setFont(QFont("Arial", 8, QFont::Normal));
    painter.setPen(TextMuted);
    painter.drawText(margin, margin - 40, QString("ReportForge Security Assessment | %1").arg(projectTitle));
    
    if (!currentSection.isEmpty()) {
        int secWidth = painter.fontMetrics().horizontalAdvance(currentSection);
        painter.drawText(width - margin - secWidth, margin - 40, currentSection);
    }
    
    painter.setPen(QPen(BorderLight, 1.5));
    painter.drawLine(margin, margin - 25, width - margin, margin - 25);

    // Footer
    painter.drawLine(margin, height - margin + 15, width - margin, height - margin + 15);
    
    painter.setFont(QFont("Arial", 8, QFont::Normal));
    painter.setPen(TextMuted);
    painter.drawText(margin, height - margin + 40, "CLASSIFICATION: CONFIDENTIAL");
    
    QString pageStr = QString("Page %1 of %2").arg(pageNum).arg(totalPages);
    painter.drawText(width - margin - painter.fontMetrics().horizontalAdvance(pageStr), height - margin + 40, pageStr);

    painter.restore();
}

// Helper to render Markdown using QTextDocument on A4 coordinates
static void renderMarkdownBlock(QPainter& painter, QPdfWriter* device, LayoutContext& ctx, const QString& projectTitle, const QString& sectionName, const QString& markdown, int& y) {
    QTextDocument doc;
    
    // Set custom stylesheet on the QTextDocument to style HTML tags
    doc.setDefaultStyleSheet(
        "body { font-family: Arial; font-size: 10pt; color: #23293b; line-height: 150%; }"
        "h1 { font-family: Arial; font-size: 14pt; color: #8b5cf6; margin-top: 15px; margin-bottom: 5px; font-weight: bold; }"
        "h2 { font-family: Arial; font-size: 11pt; color: #06b6d4; margin-top: 10px; margin-bottom: 5px; font-weight: bold; }"
        "code { font-family: 'Courier New', monospace; font-size: 9pt; background-color: #f8fafc; color: #0f172a; }"
        "ul { margin-top: 5px; margin-bottom: 5px; }"
        "li { margin-bottom: 3px; }"
    );
    
    doc.setMarkdown(markdown);
    doc.setTextWidth(ctx.width - 2 * ctx.margin);
    double docHeight = doc.size().height();

    // Check pagination
    if (y + docHeight > ctx.height - ctx.margin) {
        // Break page
        ctx.currentPage++;
        if (!ctx.dummyPass) {
            device->newPage();
            drawHeaderFooter(painter, projectTitle, sectionName, ctx.currentPage, ctx.totalPages, ctx.width, ctx.height, ctx.margin);
        }
        y = ctx.margin + 30;
    }

    if (!ctx.dummyPass) {
        painter.save();
        painter.translate(ctx.margin, y);
        doc.drawContents(&painter);
        painter.restore();
    }
    
    y += docHeight;
}

// ----------------------------------------------------
// TWO-PASS DOCUMENT LAYOUT RENDERER
// ----------------------------------------------------
static void renderDocument(QPainter& painter, QPdfWriter* device, LayoutContext& ctx, const models::Project& project, const std::vector<models::Finding>& findings, const std::vector<models::Evidence>& evidenceList) {
    QString projectTitle = QString::fromStdString(project.name);
    
    QFont coverTitleFont("Arial", 30, QFont::Bold);
    QFont coverMetaFont("Arial", 10, QFont::Normal);
    QFont h1Font("Arial", 18, QFont::Bold);
    QFont h2Font("Arial", 12, QFont::Bold);
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
        painter.setPen(QPen(QColor(139, 92, 246, 40), 1));
        for (int i = ctx.width / 10; i < ctx.width; i += ctx.width / 10) {
            painter.drawLine(i, 0, i, ctx.height);
        }
        for (int i = ctx.height / 15; i < ctx.height; i += ctx.height / 15) {
            painter.drawLine(0, i, ctx.width, i);
        }

        // Geometric banner line
        painter.setPen(QPen(AccentPurple, 14));
        painter.drawLine(ctx.margin, ctx.height * 0.22, ctx.width - ctx.margin, ctx.height * 0.22);
        painter.setPen(QPen(AccentBlue, 6));
        painter.drawLine(ctx.margin, ctx.height * 0.22 + 30, ctx.width * 0.55, ctx.height * 0.22 + 30);

        // Document title
        painter.setPen(TextWhite);
        painter.setFont(coverTitleFont);
        painter.drawText(ctx.margin, ctx.height * 0.35, "SECURITY ASSESSMENT\nREPORT");

        // Subtle logo drawing
        painter.setFont(QFont("Arial", 16, QFont::Bold));
        painter.setPen(AccentPurple);
        painter.drawText(ctx.margin, ctx.height * 0.5, "REPORTFORGE");

        // Divider
        painter.setPen(QPen(BorderLight, 1));
        painter.drawLine(ctx.margin, ctx.height * 0.54, ctx.width - ctx.margin, ctx.height * 0.54);

        // Metadata grid
        int metaY = ctx.height * 0.6;
        int col1X = ctx.margin;
        int col2X = ctx.width * 0.45;

        auto drawMetaItem = [&](int x, int y, const QString& label, const QString& value) {
            painter.setFont(coverMetaFont);
            painter.setPen(AccentBlue);
            painter.drawText(x, y, label.toUpper());
            painter.setFont(coverMetaFont);
            painter.setPen(TextWhite);
            painter.drawText(x, y + 25, value);
        };

        drawMetaItem(col1X, metaY, "Target Customer", QString::fromStdString(project.customer));
        drawMetaItem(col2X, metaY, "Engagement Project", QString::fromStdString(project.name));

        drawMetaItem(col1X, metaY + 70, "Start Date", QString::fromStdString(project.startDate));
        drawMetaItem(col2X, metaY + 70, "End Date", QString::fromStdString(project.endDate));

        drawMetaItem(col1X, metaY + 140, "Status", QString::fromStdString(core::projectStatusToString(project.status)).toUpper());
        drawMetaItem(col2X, metaY + 140, "Classification", "CONFIDENTIAL");

        // Footer block
        painter.setFont(QFont("Arial", 8, QFont::Normal));
        painter.setPen(TextMuted);
        painter.drawText(ctx.margin, ctx.height - ctx.margin, "Generated dynamically. Confidential deliverable. Property of respective client.");
        painter.restore();
    }

    // ----------------------------------------------------
    // PAGE 2: TABLE OF CONTENTS (Toc)
    // ----------------------------------------------------
    ctx.currentPage = 2;
    if (!ctx.dummyPass) {
        device->newPage();
        drawHeaderFooter(painter, projectTitle, "Table of Contents", ctx.currentPage, ctx.totalPages, ctx.width, ctx.height, ctx.margin);

        int y = ctx.margin + 60;
        painter.setFont(h1Font);
        painter.setPen(AccentPurple);
        drawDocumentIcon(painter, ctx.margin, y - 5, 25);
        painter.drawText(ctx.margin + 40, y + 18, "Table of Contents");
        y += 70;

        painter.setFont(bodyFont);
        painter.setPen(TextDark);

        auto drawTocLine = [&](const QString& title, int page) {
            QString pageStr = QString::number(page);
            int titleWidth = painter.fontMetrics().horizontalAdvance(title);
            int pageLen = painter.fontMetrics().horizontalAdvance(pageStr);
            int lineW = ctx.width - 2 * ctx.margin - titleWidth - pageLen - 40;

            painter.drawText(ctx.margin, y, title);
            
            // Draw dotted line (tab leaders)
            QString dots = "";
            int dotCount = lineW / painter.fontMetrics().horizontalAdvance(".");
            for (int k = 0; k < dotCount; ++k) dots += ".";
            
            painter.setPen(TextMuted);
            painter.drawText(ctx.margin + titleWidth + 10, y, dots);
            
            painter.setPen(TextDark);
            painter.drawText(ctx.width - ctx.margin - pageLen, y, pageStr);
            y += 35;
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
        drawHeaderFooter(painter, projectTitle, "Executive Summary", ctx.currentPage, ctx.totalPages, ctx.width, ctx.height, ctx.margin);
    }

    int y = ctx.margin + 50;
    if (!ctx.dummyPass) {
        painter.setFont(h1Font);
        painter.setPen(AccentPurple);
        drawShieldIcon(painter, ctx.margin, y - 5, 25);
        painter.drawText(ctx.margin + 40, y + 18, "1. Executive Summary");
    }
    y += 60;

    QString summaryText = QString::fromStdString(project.description);
    if (summaryText.isEmpty()) {
        summaryText = "No project description provided. This assessment was initiated to evaluate the overall security posture and identify exploitable vulnerabilities in the target system.";
    }
    renderMarkdownBlock(painter, device, ctx, projectTitle, "Executive Summary", summaryText, y);
    y += 40;

    // Title: Risk Profile Cards
    if (!ctx.dummyPass) {
        painter.setFont(h2Font);
        painter.setPen(AccentPurple);
        painter.drawText(ctx.margin, y, "Vulnerability Distribution Grid");
    }
    y += 25;

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

    // Draw Grid Cards for Severities
    int gridW = ctx.width - 2 * ctx.margin;
    int cardCount = 5;
    int spacing = 15;
    int cardW = (gridW - (cardCount - 1) * spacing) / cardCount;
    int cardH = 110;

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
            painter.setPen(QPen(sColor, 1.5));
            painter.setBrush(QBrush(getSeverityBgColor(cardEnums[i])));
            painter.drawRoundedRect(cx, y, cardW, cardH, 6, 6);

            // Title
            painter.setFont(QFont("Arial", 8, QFont::Bold));
            painter.setPen(sColor);
            painter.drawText(QRect(cx, y + 15, cardW, 20), Qt::AlignCenter, cards[i].first);

            // Number value
            painter.setFont(QFont("Arial", 22, QFont::Bold));
            painter.setPen(sColor);
            painter.drawText(QRect(cx, y + 40, cardW, 40), Qt::AlignCenter, QString::number(cards[i].second));
        }
        painter.restore();
    }
    y += cardH + 50;

    // Methodology Section
    if (!ctx.dummyPass) {
        painter.setFont(h2Font);
        painter.setPen(AccentPurple);
        painter.drawText(ctx.margin, y, "Methodology & Scope");
    }
    y += 20;

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
        drawHeaderFooter(painter, projectTitle, "Scope & Timeline", ctx.currentPage, ctx.totalPages, ctx.width, ctx.height, ctx.margin);
    }

    y = ctx.margin + 50;
    if (!ctx.dummyPass) {
        painter.setFont(h1Font);
        painter.setPen(AccentPurple);
        drawTargetIcon(painter, ctx.margin, y - 5, 25);
        painter.drawText(ctx.margin + 40, y + 18, "2. Assessment Scope & Timeline");
    }
    y += 60;

    QString scopeTargets = "**Targets:**\n\n" + QString::fromStdString(project.targets) + "\n\n"
                           "**Scope constraints:**\n\n" + QString::fromStdString(project.scope);
    renderMarkdownBlock(painter, device, ctx, projectTitle, "Scope & Timeline", scopeTargets, y);
    y += 60;

    // Timeline Graphic Section
    if (!ctx.dummyPass) {
        painter.setFont(h2Font);
        painter.setPen(AccentPurple);
        drawCalendarIcon(painter, ctx.margin, y - 5, 20);
        painter.drawText(ctx.margin + 30, y + 14, "Assessment Timeline");
    }
    y += 40;

    // Draw stylized horizontal timeline axis
    if (!ctx.dummyPass) {
        painter.save();
        painter.setRenderHint(QPainter::Antialiasing);

        int timelineW = ctx.width - 2 * ctx.margin - 80;
        int tY = y + 30;
        
        painter.setPen(QPen(BorderLight, 3));
        painter.drawLine(ctx.margin + 40, tY, ctx.margin + 40 + timelineW, tY);

        std::vector<QString> steps = {"Planning", "Recon", "Scanning", "Exploitation", "Reporting"};
        int stepSpacing = timelineW / (steps.size() - 1);

        for (size_t i = 0; i < steps.size(); ++i) {
            int cx = ctx.margin + 40 + i * stepSpacing;
            
            // Draw dot
            painter.setPen(QPen(AccentPurple, 3));
            painter.setBrush(AccentBlue);
            painter.drawEllipse(cx - 8, tY - 8, 16, 16);

            // Step Label
            painter.setFont(QFont("Arial", 8, QFont::Bold));
            painter.setPen(TextDark);
            int labelW = 120;
            painter.drawText(QRect(cx - labelW/2, tY + 15, labelW, 20), Qt::AlignCenter, steps[i]);
        }
        painter.restore();
    }
    y += 80;

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
            drawHeaderFooter(painter, projectTitle, "Detailed Findings", ctx.currentPage, ctx.totalPages, ctx.width, ctx.height, ctx.margin);
        }

        y = ctx.margin + 50;

        // Render Finding header with colored severity stripe
        if (!ctx.dummyPass) {
            painter.save();
            painter.fillRect(ctx.margin, y, ctx.width - 2 * ctx.margin, 40, getSeverityColor(f.severity));
            
            painter.setFont(QFont("Arial", 11, QFont::Bold));
            painter.setPen(TextWhite);
            QString sevLabel = QString("  [%1] %2").arg(QString::fromStdString(core::severityToString(f.severity)).toUpper(), findingSectionTitle);
            painter.drawText(QRect(ctx.margin, y, ctx.width - 2 * ctx.margin, 40), Qt::AlignVCenter | Qt::AlignLeft, sevLabel);
            painter.restore();
        }
        y += 65;

        // Metadata clean HTML table
        QString htmlTable = QString(
            "<table cellpadding='6' cellspacing='0' style='border-collapse: collapse; width: 100%; border: 1px solid #cbd5e1; font-family: Arial; font-size: 9.5pt; color: #1e293b;'>"
            "  <tr style='background-color: #f8fafc;'>"
            "    <th style='border: 1px solid #cbd5e1; text-align: left; width: 30%%; font-weight: bold; color: #475569;'>Attribute</th>"
            "    <th style='border: 1px solid #cbd5e1; text-align: left; width: 70%%; font-weight: bold; color: #475569;'>Detail Value</th>"
            "  </tr>"
            "  <tr>"
            "    <td style='border: 1px solid #cbd5e1; font-weight: bold;'>Severity & Rating</td>"
            "    <td style='border: 1px solid #cbd5e1; color: %1; font-weight: bold;'>%2 (CVSS: %3)</td>"
            "  </tr>"
            "  <tr>"
            "    <td style='border: 1px solid #cbd5e1; font-weight: bold;'>Classification</td>"
            "    <td style='border: 1px solid #cbd5e1;'>%4 | %5</td>"
            "  </tr>"
            "  <tr>"
            "    <td style='border: 1px solid #cbd5e1; font-weight: bold;'>Lifecycle Status</td>"
            "    <td style='border: 1px solid #cbd5e1;'>%6</td>"
            "  </tr>"
            "  <tr>"
            "    <td style='border: 1px solid #cbd5e1; font-weight: bold;'>Affected Targets</td>"
            "    <td style='border: 1px solid #cbd5e1; font-family: monospace;'>%7</td>"
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
        tableDoc.setTextWidth(ctx.width - 2 * ctx.margin);
        tableDoc.setHtml(htmlTable);
        double tableH = tableDoc.size().height();

        if (!ctx.dummyPass) {
            painter.save();
            painter.translate(ctx.margin, y);
            tableDoc.drawContents(&painter);
            painter.restore();
        }
        y += tableH + 30;

        // Subsections: Description, Impact, Recommendation
        auto renderSubsection = [&](const QString& name, const QString& mdContent, const QString& iconType) {
            if (mdContent.isEmpty()) return;

            if (y + 120 > ctx.height - ctx.margin) {
                ctx.currentPage++;
                if (!ctx.dummyPass) {
                    device->newPage();
                    drawHeaderFooter(painter, projectTitle, "Detailed Findings", ctx.currentPage, ctx.totalPages, ctx.width, ctx.height, ctx.margin);
                }
                y = ctx.margin + 30;
            }

            if (!ctx.dummyPass) {
                painter.setFont(h2Font);
                painter.setPen(AccentPurple);
                if (iconType == "bug") drawBugIcon(painter, ctx.margin, y - 5, 20);
                else if (iconType == "warning") drawWarningIcon(painter, ctx.margin, y - 5, 20);
                else if (iconType == "shield") drawShieldIcon(painter, ctx.margin, y - 5, 20);
                
                painter.drawText(ctx.margin + 30, y + 14, name);
            }
            y += 25;

            renderMarkdownBlock(painter, device, ctx, projectTitle, "Detailed Findings", mdContent, y);
            y += 20;
        };

        renderSubsection("Vulnerability Description", QString::fromStdString(f.description), "bug");
        renderSubsection("Risk & Exploit Impact", QString::fromStdString(f.impact), "warning");
        renderSubsection("Remediation Recommendation", QString::fromStdString(f.recommendation), "shield");

        // Code block for Proof of Concept (Monospace card with grey background)
        if (!f.proofOfConcept.empty()) {
            if (y + 100 > ctx.height - ctx.margin) {
                ctx.currentPage++;
                if (!ctx.dummyPass) {
                    device->newPage();
                    drawHeaderFooter(painter, projectTitle, "Detailed Findings", ctx.currentPage, ctx.totalPages, ctx.width, ctx.height, ctx.margin);
                }
                y = ctx.margin + 30;
            }

            if (!ctx.dummyPass) {
                painter.setFont(h2Font);
                painter.setPen(AccentPurple);
                drawDocumentIcon(painter, ctx.margin, y - 5, 20);
                painter.drawText(ctx.margin + 30, y + 14, "Proof of Concept / Technical Payload");
            }
            y += 25;

            // Formulate Code Block text layout
            QTextDocument codeDoc;
            codeDoc.setDefaultFont(monoFont);
            codeDoc.setTextWidth(ctx.width - 2 * ctx.margin - 40); // add padding
            codeDoc.setPlainText(QString::fromStdString(f.proofOfConcept));
            double codeH = codeDoc.size().height();

            if (y + codeH + 30 > ctx.height - ctx.margin) {
                ctx.currentPage++;
                if (!ctx.dummyPass) {
                    device->newPage();
                    drawHeaderFooter(painter, projectTitle, "Detailed Findings", ctx.currentPage, ctx.totalPages, ctx.width, ctx.height, ctx.margin);
                }
                y = ctx.margin + 30;
            }

            if (!ctx.dummyPass) {
                painter.save();
                // Draw rounded background block
                painter.setPen(QPen(BorderLight, 1.5));
                painter.setBrush(QBrush(CodeBg));
                painter.drawRoundedRect(ctx.margin, y, ctx.width - 2 * ctx.margin, codeH + 20, 6, 6);
                
                // Draw code content
                painter.translate(ctx.margin + 20, y + 10);
                codeDoc.drawContents(&painter);
                painter.restore();
            }
            y += codeH + 40;
        }

        // Screenshots and Images embedding
        std::vector<models::Evidence> fEv;
        for (const auto& ev : evidenceList) {
            if (ev.findingId == f.id && ev.fileType == core::MediaType::Image) {
                fEv.push_back(ev);
            }
        }

        if (!fEv.empty()) {
            if (y + 60 > ctx.height - ctx.margin) {
                ctx.currentPage++;
                if (!ctx.dummyPass) {
                    device->newPage();
                    drawHeaderFooter(painter, projectTitle, "Detailed Findings", ctx.currentPage, ctx.totalPages, ctx.width, ctx.height, ctx.margin);
                }
                y = ctx.margin + 30;
            }

            if (!ctx.dummyPass) {
                painter.setFont(h2Font);
                painter.setPen(AccentPurple);
                drawImageIcon(painter, ctx.margin, y - 5, 20);
                painter.drawText(ctx.margin + 30, y + 14, "Vulnerability Evidence / Screenshots");
            }
            y += 35;
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
            if (y + targetH + 60 > ctx.height - ctx.margin) {
                ctx.currentPage++;
                if (!ctx.dummyPass) {
                    device->newPage();
                    drawHeaderFooter(painter, projectTitle, "Detailed Findings", ctx.currentPage, ctx.totalPages, ctx.width, ctx.height, ctx.margin);
                }
                y = ctx.margin + 30;
            }

            if (!ctx.dummyPass) {
                painter.save();
                painter.setRenderHint(QPainter::Antialiasing);
                
                // Draw centered image
                int imgX = ctx.margin + (maxW - targetW) / 2;
                painter.drawImage(QRect(imgX, y, targetW, targetH), img);

                // Draw thin picture outline border
                painter.setPen(QPen(BorderLight, 1));
                painter.setBrush(Qt::NoBrush);
                painter.drawRect(imgX, y, targetW, targetH);
                
                // Draw caption
                painter.setFont(QFont("Arial", 8, QFont::StyleItalic));
                painter.setPen(TextMuted);
                QString caption = QString("Figure %1 - %2").arg(figCounter).arg(QString::fromStdString(ev.fileName));
                
                int capW = painter.fontMetrics().horizontalAdvance(caption);
                painter.drawText(ctx.margin + (maxW - capW) / 2, y + targetH + 20, caption);
                painter.restore();
            }
            y += targetH + 40;
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
        drawHeaderFooter(painter, projectTitle, "Appendix", ctx.currentPage, ctx.totalPages, ctx.width, ctx.height, ctx.margin);
    }

    y = ctx.margin + 50;
    if (!ctx.dummyPass) {
        painter.setFont(h1Font);
        painter.setPen(AccentPurple);
        drawDocumentIcon(painter, ctx.margin, y - 5, 25);
        painter.drawText(ctx.margin + 40, y + 18, "Appendix: Threat Classifications");
    }
    y += 60;

    QString appendixTable = 
        "<table cellpadding='6' cellspacing='0' style='border-collapse: collapse; width: 100%; border: 1px solid #cbd5e1; font-family: Arial; font-size: 9pt; color: #1e293b;'>"
        "  <tr style='background-color: #f8fafc; font-weight: bold; color: #475569;'>"
        "    <th style='border: 1px solid #cbd5e1; text-align: left; width: 25%%;'>Rating</th>"
        "    <th style='border: 1px solid #cbd5e1; text-align: left; width: 20%%;'>CVSS Score</th>"
        "    <th style='border: 1px solid #cbd5e1; text-align: left; width: 55%%;'>Threat Description</th>"
        "  </tr>"
        "  <tr>"
        "    <td style='border: 1px solid #cbd5e1; color: #991b1b; font-weight: bold;'>CRITICAL</td>"
        "    <td style='border: 1px solid #cbd5e1; font-weight: bold;'>9.0 - 10.0</td>"
        "    <td style='border: 1px solid #cbd5e1;'>Exploitation leads to remote shell access, full domain takeover, or database exfiltration.</td>"
        "  </tr>"
        "  <tr>"
        "    <td style='border: 1px solid #cbd5e1; color: #dc2626; font-weight: bold;'>HIGH</td>"
        "    <td style='border: 1px solid #cbd5e1; font-weight: bold;'>7.0 - 8.9</td>"
        "    <td style='border: 1px solid #cbd5e1;'>Exploitation allows sensitive data reading, local user takeover, or access bypass.</td>"
        "  </tr>"
        "  <tr>"
        "    <td style='border: 1px solid #cbd5e1; color: #d97706; font-weight: bold;'>MEDIUM</td>"
        "    <td style='border: 1px solid #cbd5e1; font-weight: bold;'>4.0 - 6.9</td>"
        "    <td style='border: 1px solid #cbd5e1;'>Vulnerability requires user interaction (e.g. CSRF, XSS) or specific configs.</td>"
        "  </tr>"
        "  <tr>"
        "    <td style='border: 1px solid #cbd5e1; color: #2563eb; font-weight: bold;'>LOW</td>"
        "    <td style='border: 1px solid #cbd5e1; font-weight: bold;'>0.1 - 3.9</td>"
        "    <td style='border: 1px solid #cbd5e1;'>Hardening details or information disclosures. Minimal threat profile.</td>"
        "  </tr>"
        "</table>";

    QTextDocument appTableDoc;
    appTableDoc.setTextWidth(ctx.width - 2 * ctx.margin);
    appTableDoc.setHtml(appendixTable);
    double appTableH = appTableDoc.size().height();

    if (!ctx.dummyPass) {
        painter.save();
        painter.translate(ctx.margin, y);
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
    // PASS 1: GATHER PAGE METRICS ON A BUFFER DEVICE
    QByteArray bufferBytes;
    QBuffer bufferDevice(&bufferBytes);
    bufferDevice.open(QIODevice::WriteOnly);
    
    QPdfWriter dummyWriter(&bufferDevice);
    dummyWriter.setPageSize(QPageSize(QPageSize::A4));
    dummyWriter.setResolution(300);
    
    LayoutContext ctx;
    ctx.dummyPass = true;
    ctx.currentPage = 1;
    ctx.width = dummyWriter.width();
    ctx.height = dummyWriter.height();
    ctx.margin = ctx.width * 0.08;

    {
        QPainter dummyPainter(&dummyWriter);
        renderDocument(dummyPainter, &dummyWriter, ctx, project, findings, evidenceList);
    }
    
    int finalTotalPages = ctx.currentPage;
    auto tocMap = ctx.tocEntries;
    bufferDevice.close();

    // PASS 2: RENDER TRUE PRINT DOCUMENT TO THE ACTUAL TARGET FILE PATH
    QPdfWriter realWriter(QString::fromStdString(outputPath));
    realWriter.setPageSize(QPageSize(QPageSize::A4));
    realWriter.setResolution(300);
    realWriter.setPageMargins(QMarginsF(20, 20, 20, 20), QPageLayout::Millimeter);

    LayoutContext realCtx;
    realCtx.dummyPass = false;
    realCtx.currentPage = 1;
    realCtx.totalPages = finalTotalPages;
    realCtx.tocEntries = tocMap;
    realCtx.width = realWriter.width();
    realCtx.height = realWriter.height();
    realCtx.margin = realCtx.width * 0.08;

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

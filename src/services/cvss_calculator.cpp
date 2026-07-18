#include "cvss_calculator.h"
#include <cmath>
#include <algorithm>

namespace reportforge::services {

// Helper to round up to one decimal place according to CVSS specs (round up to nearest 0.1)
static double roundUp1(double val) {
    double intVal = std::round(val * 100000.0);
    int intValModulo = static_cast<int>(intVal) % 10000;
    if (intValModulo == 0) {
        return val;
    }
    return std::ceil(val * 10.0) / 10.0;
}

double CvssCalculator::calculate(const Metrics& m) {
    // 1. Map Attack Vector (AV)
    double av = 0.85; // Network
    if (m.attackVector == 1) av = 0.62;      // Adjacent
    else if (m.attackVector == 2) av = 0.55; // Local
    else if (m.attackVector == 3) av = 0.20; // Physical

    // 2. Map Attack Complexity (AC)
    double ac = (m.attackComplexity == 0) ? 0.77 : 0.44; // Low / High

    // 3. Map Privileges Required (PR)
    double pr = 0.85; // None
    if (m.scope == 0) { // Scope Unchanged
        if (m.privilegesRequired == 1) pr = 0.62;      // Low
        else if (m.privilegesRequired == 2) pr = 0.27; // High
    } else { // Scope Changed
        if (m.privilegesRequired == 1) pr = 0.68;      // Low
        else if (m.privilegesRequired == 2) pr = 0.50; // High
    }

    // 4. Map User Interaction (UI)
    double ui = (m.userInteraction == 0) ? 0.85 : 0.62; // None / Required

    // 5. Map Confidentiality (C)
    double c = 0.56; // High
    if (m.confidentiality == 1) c = 0.22;      // Low
    else if (m.confidentiality == 2) c = 0.0;  // None

    // 6. Map Integrity (I)
    double i = 0.56; // High
    if (m.integrity == 1) i = 0.22;      // Low
    else if (m.integrity == 2) i = 0.0;  // None

    // 7. Map Availability (A)
    double a = 0.56; // High
    if (m.availability == 1) a = 0.22;      // Low
    else if (m.availability == 2) a = 0.0;  // None

    // Calculations
    double exploitability = 8.22 * av * ac * pr * ui;
    double iscBase = 1.0 - (1.0 - c) * (1.0 - i) * (1.0 - a);
    double impact = 0.0;

    if (m.scope == 0) {
        impact = 6.42 * iscBase;
    } else {
        impact = 7.52 * (iscBase - 0.029) - 3.25 * std::pow(iscBase - 0.02, 15);
    }

    if (impact <= 0) {
        return 0.0;
    }

    double baseScore = 0.0;
    if (m.scope == 0) {
        baseScore = std::min(impact + exploitability, 10.0);
    } else {
        baseScore = std::min(1.08 * (impact + exploitability), 10.0);
    }

    return roundUp1(baseScore);
}

std::string CvssCalculator::toStringVector(const Metrics& m) {
    std::string vec = "CVSS:3.1";

    // AV
    if (m.attackVector == 0) vec += "/AV:N";
    else if (m.attackVector == 1) vec += "/AV:A";
    else if (m.attackVector == 2) vec += "/AV:L";
    else if (m.attackVector == 3) vec += "/AV:P";

    // AC
    if (m.attackComplexity == 0) vec += "/AC:L";
    else vec += "/AC:H";

    // PR
    if (m.privilegesRequired == 0) vec += "/PR:N";
    else if (m.privilegesRequired == 1) vec += "/PR:L";
    else if (m.privilegesRequired == 2) vec += "/PR:H";

    // UI
    if (m.userInteraction == 0) vec += "/UI:N";
    else vec += "/UI:R";

    // S
    if (m.scope == 0) vec += "/S:U";
    else vec += "/S:C";

    // C
    if (m.confidentiality == 0) vec += "/C:H";
    else if (m.confidentiality == 1) vec += "/C:L";
    else vec += "/C:N";

    // I
    if (m.integrity == 0) vec += "/I:H";
    else if (m.integrity == 1) vec += "/I:L";
    else vec += "/I:N";

    // A
    if (m.availability == 0) vec += "/A:H";
    else if (m.availability == 1) vec += "/A:L";
    else vec += "/A:N";

    return vec;
}

} // namespace reportforge::services

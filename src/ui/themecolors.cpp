#include "themecolors.h"

#include <QColor>
#include <QHash>

namespace {

struct ThemeData {
    QHash<QString, QString> qss;
    QHash<QString, QString> ui;
};

const ThemeData& themeData(bool dark) {
    static const ThemeData darkTheme{
        {
            {"@D01@", "#1f1f1f"}, {"@D02@", "#242424"}, {"@D03@", "#292929"}, {"@D04@", "#2a2a2a"},
            {"@D05@", "#303030"}, {"@D06@", "#323232"}, {"@D07@", "#333333"}, {"@D08@", "#343434"},
            {"@D09@", "#3c3c3c"}, {"@D10@", "#4a4a4a"}, {"@D11@", "#555555"}, {"@D12@", "#5f5f5f"},
            {"@D13@", "#717171"}, {"@D14@", "#777777"}, {"@D15@", "#7a7a7a"}, {"@D16@", "#8a8a8a"},
            {"@D17@", "#999999"}, {"@D18@", "#9a9a9a"}, {"@D19@", "#aaaaaa"}, {"@D20@", "#b5b5b5"},
            {"@D21@", "#bebebe"}, {"@D22@", "#e6e6e6"}, {"@D23@", "#ffffff"},
        },
        {
            {"palette_window", "#1f1f1f"}, {"palette_text", "#e6e6e6"}, {"palette_base", "#2a2a2a"},
            {"palette_alt_base", "#292929"}, {"palette_button", "#5f5f5f"}, {"palette_highlight", "#5f5f5f"},
            {"palette_tooltip_base", "#2a2a2a"}, {"palette_tooltip_text", "#e6e6e6"}, {"white", "#ffffff"},
            {"status_bg", "#2a2a2a"}, {"status_fg", "#b5b5b5"}, {"theme_btn_bg", "#5f5f5f"},
            {"theme_btn_hover", "#717171"},

            {"calc_num_bg", "#505050"}, {"calc_num_fg", "#f0f0f0"}, {"calc_num_hover", "#686868"},
            {"calc_op_bg", "#707070"}, {"calc_op_hover", "#888888"},
            {"calc_bit_bg", "#5a5a5a"}, {"calc_bit_fg", "#e0e0e0"}, {"calc_bit_hover", "#727272"},
            {"calc_func_bg", "#484848"}, {"calc_func_fg", "#cccccc"}, {"calc_func_hover", "#606060"},
            {"calc_hex_bg", "#606060"}, {"calc_hex_fg", "#f0f0f0"}, {"calc_hex_hover", "#787878"},
            {"calc_hex_disabled_bg", "#3a3a3a"}, {"calc_hex_disabled_fg", "#666666"},
            {"calc_eq_bg", "#787878"}, {"calc_eq_hover", "#909090"},
            {"calc_clear_bg", "#383838"}, {"calc_clear_fg", "#cccccc"}, {"calc_clear_hover", "#505050"},
            {"calc_display_bg", "#444444"}, {"calc_display_fg", "#f0f0f0"}, {"calc_display_border", "#666666"},
            {"calc_expr_fg", "#9a9a9a"}, {"calc_hint_fg", "#7f7f7f"},

            {"group_title", "#b5b5b5"}, {"group_border", "#444444"},
            {"field_label", "#aaaaaa"}, {"input_bg", "#444444"}, {"input_fg", "#f0f0f0"},
            {"input_border", "#666666"}, {"value_bg", "#444444"}, {"value_fg", "#f0f0f0"},
            {"value_border", "#666666"}, {"formula_fg", "#888888"},
            {"bit_on_bg", "#707070"}, {"bit_on_fg", "#ffffff"}, {"bit_on_border", "#999999"}, {"bit_on_hover", "#888888"},
            {"bit_off_bg", "#484848"}, {"bit_off_fg", "#cccccc"}, {"bit_off_border", "#666666"}, {"bit_off_hover", "#606060"},
        }
    };

    static const ThemeData lightTheme{
        {
            {"@L01@", "#004fa0"}, {"@L02@", "#006600"}, {"@L03@", "#0066cc"}, {"@L04@", "#0077dd"},
            {"@L05@", "#1a1a2e"}, {"@L06@", "#2a3a80"}, {"@L07@", "#2d4a88"}, {"@L08@", "#3d5aaa"},
            {"@L09@", "#4a6abf"}, {"@L10@", "#4d6abf"}, {"@L11@", "#666666"}, {"@L12@", "#6670a0"},
            {"@L13@", "#888888"}, {"@L14@", "#8890b0"}, {"@L15@", "#8899cc"}, {"@L16@", "#c5cbdd"},
            {"@L17@", "#d8ddf0"}, {"@L18@", "#dde1ee"}, {"@L19@", "#dde3f5"}, {"@L20@", "#e8eaf5"},
            {"@L21@", "#eaecf5"}, {"@L22@", "#f4f6fb"}, {"@L23@", "#f7f8fd"}, {"@L24@", "#ffffff"},
        },
        {
            {"palette_window", "#f4f6fb"}, {"palette_text", "#1a1a2e"}, {"palette_base", "#ffffff"},
            {"palette_alt_base", "#f7f8fd"}, {"palette_button", "#3d5aaa"}, {"palette_highlight", "#3d5aaa"},
            {"palette_tooltip_base", "#ffffff"}, {"palette_tooltip_text", "#1a1a2e"}, {"white", "#ffffff"},
            {"status_bg", "#eaecf5"}, {"status_fg", "#3d5aaa"}, {"theme_btn_bg", "#3d5aaa"},
            {"theme_btn_hover", "#4d6abf"},

            {"calc_num_bg", "#e4e8f5"}, {"calc_num_fg", "#1a1a2e"}, {"calc_num_hover", "#d0d4e8"},
            {"calc_op_bg", "#3d5aaa"}, {"calc_op_hover", "#4d6abf"},
            {"calc_bit_bg", "#c8cde0"}, {"calc_bit_fg", "#1a1a2e"}, {"calc_bit_hover", "#b8bdd8"},
            {"calc_func_bg", "#d8dcee"}, {"calc_func_fg", "#1a1a2e"}, {"calc_func_hover", "#c8cce0"},
            {"calc_hex_bg", "#dce0f0"}, {"calc_hex_fg", "#1a1a2e"}, {"calc_hex_hover", "#ccd0e8"},
            {"calc_hex_disabled_bg", "#eaecf5"}, {"calc_hex_disabled_fg", "#9099bb"},
            {"calc_eq_bg", "#2d4a88"}, {"calc_eq_hover", "#3d5aaa"},
            {"calc_clear_bg", "#f5e8e8"}, {"calc_clear_fg", "#c0392b"}, {"calc_clear_hover", "#ead8d8"},
            {"calc_display_bg", "#ffffff"}, {"calc_display_fg", "#1a1a2e"}, {"calc_display_border", "#c5cbdd"},
            {"calc_expr_fg", "#5566aa"}, {"calc_hint_fg", "#9099bb"},

            {"group_title", "#3d5aaa"}, {"group_border", "#c5cbdd"},
            {"field_label", "#3d5aaa"}, {"input_bg", "#ffffff"}, {"input_fg", "#1a1a2e"},
            {"input_border", "#c5cbdd"}, {"value_bg", "#f0f2fa"}, {"value_fg", "#1a1a2e"},
            {"value_border", "#c5cbdd"}, {"formula_fg", "#6670a0"},
            {"bit_on_bg", "#3d5aaa"}, {"bit_on_fg", "#ffffff"}, {"bit_on_border", "#8899cc"}, {"bit_on_hover", "#4d6abf"},
            {"bit_off_bg", "#eaecf5"}, {"bit_off_fg", "#4455aa"}, {"bit_off_border", "#c5cbdd"}, {"bit_off_hover", "#d8dcee"},
        }
    };

    return dark ? darkTheme : lightTheme;
}

QString c(bool dark, const QString& key) {
    return themeData(dark).ui.value(key);
}

} // namespace

namespace ThemeColors {

QString applyQssColors(QString qssTemplate, bool dark) {
    const auto& map = themeData(dark).qss;
    for (auto it = map.constBegin(); it != map.constEnd(); ++it)
        qssTemplate.replace(it.key(), it.value());
    return qssTemplate;
}

QPalette applicationPalette(bool dark) {
    QPalette p;
    p.setColor(QPalette::Window, QColor(c(dark, "palette_window")));
    p.setColor(QPalette::WindowText, QColor(c(dark, "palette_text")));
    p.setColor(QPalette::Base, QColor(c(dark, "palette_base")));
    p.setColor(QPalette::AlternateBase, QColor(c(dark, "palette_alt_base")));
    p.setColor(QPalette::Text, QColor(c(dark, "palette_text")));
    p.setColor(QPalette::Button, QColor(c(dark, "palette_button")));
    p.setColor(QPalette::ButtonText, QColor(c(dark, "white")));
    p.setColor(QPalette::Highlight, QColor(c(dark, "palette_highlight")));
    p.setColor(QPalette::HighlightedText, QColor(c(dark, "white")));
    p.setColor(QPalette::ToolTipBase, QColor(c(dark, "palette_tooltip_base")));
    p.setColor(QPalette::ToolTipText, QColor(c(dark, "palette_tooltip_text")));
    return p;
}

QString statusBarStyle(bool dark) {
    return QString("background:%1;color:%2;font-size:11px;")
        .arg(c(dark, "status_bg"), c(dark, "status_fg"));
}

QString themeToggleButtonStyle(bool dark) {
    return QString("QPushButton{background:%1;color:%2;border:none;border-radius:4px;padding:4px 10px;font-size:12px;}"
                   "QPushButton:hover{background:%3;}")
        .arg(c(dark, "theme_btn_bg"), c(dark, "white"), c(dark, "theme_btn_hover"));
}

QString calcNumButton(bool dark) {
    return QString("QPushButton{background:%1;color:%2;font-size:13px;border-radius:4px;padding:5px;}"
                   "QPushButton:hover{background:%3;}")
        .arg(c(dark, "calc_num_bg"), c(dark, "calc_num_fg"), c(dark, "calc_num_hover"));
}

QString calcOpButton(bool dark) {
    return QString("QPushButton{background:%1;color:%2;font-size:13px;border-radius:4px;padding:5px;}"
                   "QPushButton:hover{background:%3;}")
        .arg(c(dark, "calc_op_bg"), c(dark, "white"), c(dark, "calc_op_hover"));
}

QString calcBitButton(bool dark) {
    return QString("QPushButton{background:%1;color:%2;font-size:11px;border-radius:4px;padding:4px;}"
                   "QPushButton:hover{background:%3;}")
        .arg(c(dark, "calc_bit_bg"), c(dark, "calc_bit_fg"), c(dark, "calc_bit_hover"));
}

QString calcFuncButton(bool dark) {
    return QString("QPushButton{background:%1;color:%2;font-size:11px;border-radius:4px;padding:4px;}"
                   "QPushButton:hover{background:%3;}")
        .arg(c(dark, "calc_func_bg"), c(dark, "calc_func_fg"), c(dark, "calc_func_hover"));
}

QString calcHexButton(bool dark) {
    return QString("QPushButton{background:%1;color:%2;font-size:13px;border-radius:4px;padding:5px;}"
                   "QPushButton:hover{background:%3;}QPushButton:disabled{background:%4;color:%5;}")
        .arg(c(dark, "calc_hex_bg"), c(dark, "calc_hex_fg"), c(dark, "calc_hex_hover"),
             c(dark, "calc_hex_disabled_bg"), c(dark, "calc_hex_disabled_fg"));
}

QString calcEqButton(bool dark) {
    return QString("QPushButton{background:%1;color:%2;font-size:13px;border-radius:4px;padding:5px;}"
                   "QPushButton:hover{background:%3;}")
        .arg(c(dark, "calc_eq_bg"), c(dark, "white"), c(dark, "calc_eq_hover"));
}

QString calcClearButton(bool dark) {
    return QString("QPushButton{background:%1;color:%2;font-size:11px;border-radius:4px;padding:4px;}"
                   "QPushButton:hover{background:%3;}")
        .arg(c(dark, "calc_clear_bg"), c(dark, "calc_clear_fg"), c(dark, "calc_clear_hover"));
}

QString calcSecondFuncButton(bool dark) {
    if (dark) {
        // Dark theme: keep clear button style
        return QString("QPushButton{background:%1;color:%2;font-size:11px;border-radius:4px;padding:4px;}"
                       "QPushButton:hover{background:%3;}")
            .arg(c(dark, "calc_clear_bg"), c(dark, "calc_clear_fg"), c(dark, "calc_clear_hover"));
    } else {
        // Light theme: use bit button style (same as ROL)
        return QString("QPushButton{background:%1;color:%2;font-size:11px;border-radius:4px;padding:4px;}"
                       "QPushButton:hover{background:%3;}")
            .arg(c(dark, "calc_bit_bg"), c(dark, "calc_bit_fg"), c(dark, "calc_bit_hover"));
    }
}

QString calcSecondFuncButtonActive(bool dark) {
    // "Pressed" look: darker background, highlighted border
    if (dark) {
        return QString("QPushButton{background:#606060;color:#ffffff;font-size:11px;border-radius:4px;padding:4px;"
                       "border:2px solid #888888;}"
                       "QPushButton:hover{background:#707070;}");
    } else {
        return QString("QPushButton{background:#3d5aaa;color:#ffffff;font-size:11px;border-radius:4px;padding:4px;"
                       "border:2px solid #6080cc;}"
                       "QPushButton:hover{background:#4d6abf;}");
    }
}

QString calcDisplayStyle(bool dark) {
    return QString("background:%1;color:%2;font-size:22px;font-family:'Consolas','Courier New',monospace;"
                   "border:1px solid %3;border-radius:4px;padding:4px 8px;")
        .arg(c(dark, "calc_display_bg"), c(dark, "calc_display_fg"), c(dark, "calc_display_border"));
}

QString calcExprStyle(bool dark) {
    return QString("color:_%1;font-size:12px;padding:2px 6px;").arg(c(dark, "calc_expr_fg"));
}

QString calcHintStyle(bool dark) {
    return QString("color:%1;font-size:10px;padding:2px 4px;").arg(c(dark, "calc_hint_fg"));
}

QString baseBitButtonStyle(bool dark, bool on) {
    const QString bg = c(dark, on ? "bit_on_bg" : "bit_off_bg");
    const QString fg = c(dark, on ? "bit_on_fg" : "bit_off_fg");
    const QString border = c(dark, on ? "bit_on_border" : "bit_off_border");
    const QString hover = c(dark, on ? "bit_on_hover" : "bit_off_hover");
    return QString("QPushButton{background:%1;color:%2;font-size:8px;%3border-radius:3px;border:1px solid %4;padding:0px;"
                   "min-width:14px;max-width:14px;min-height:14px;max-height:14px;}"
                   "QPushButton:hover{background:%5;}")
        .arg(bg, fg, on ? "font-weight:bold;" : "", border, hover);
}

QString baseGroupStyle(bool dark) {
    return QString("QGroupBox{color:%1;font-size:13px;font-weight:bold;border:1px solid %2;border-radius:6px;"
                   "margin-top:8px;padding-top:8px;}QGroupBox::title{subcontrol-origin:margin;left:10px;}")
        .arg(c(dark, "group_title"), c(dark, "group_border"));
}

QString baseFieldLabelStyle(bool dark) {
    return QString("color:%1;font-size:13px;font-weight:bold;").arg(c(dark, "field_label"));
}

QString baseEditStyle(bool dark) {
    return QString("background:%1;color:%2;font-family:'Consolas','Courier New',monospace;font-size:16px;"
                   "border:1px solid %3;border-radius:4px;padding:4px 8px;")
        .arg(c(dark, "input_bg"), c(dark, "input_fg"), c(dark, "input_border"));
}

QString baseValueStyle(bool dark) {
    return QString("background:%1;color:%2;font-family:'Consolas';font-size:13px;border:1px solid %3;"
                   "border-radius:3px;padding:2px 8px;")
        .arg(c(dark, "value_bg"), c(dark, "value_fg"), c(dark, "value_border"));
}

QString unitGroupStyle(bool dark) {
    return QString("QGroupBox{color:%1;font-size:13px;font-weight:bold;border:1px solid %2;border-radius:6px;"
                   "margin-top:8px;padding-top:10px;}QGroupBox::title{subcontrol-origin:margin;left:10px;}")
        .arg(c(dark, "group_title"), c(dark, "group_border"));
}

QString unitFieldStyle(bool dark) {
    return QString("background:%1;color:%2;font-family:'Consolas';font-size:16px;border:1px solid %3;"
                   "border-radius:4px;padding:4px 6px;")
        .arg(c(dark, "input_bg"), c(dark, "input_fg"), c(dark, "input_border"));
}

QString unitResultStyle(bool dark) {
    return QString("background:%1;color:%2;font-family:'Consolas';font-size:16px;border:1px solid %3;"
                   "border-radius:4px;padding:4px 6px;min-width:96px;")
        .arg(c(dark, "value_bg"), c(dark, "value_fg"), c(dark, "value_border"));
}

QString unitTitleStyle(bool dark) {
    return QString("color:%1;font-size:16px;font-weight:bold;").arg(c(dark, "group_title"));
}

QString unitFormulaStyle(bool dark) {
    return QString("color:%1;font-size:12px;padding:4px;").arg(c(dark, "formula_fg"));
}

} // namespace ThemeColors


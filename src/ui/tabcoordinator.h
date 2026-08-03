#pragma once

#include <QObject>
#include <QPair>
#include <QString>
#include <QVector>

class QMenu;
class QShortcut;
class QTabWidget;
class QWidget;

class TabCoordinator : public QObject {
    Q_OBJECT

public:
    explicit TabCoordinator(QTabWidget* tabWidget, QObject* parent = nullptr);

    void setExtraPages(const QVector<QPair<QString, QWidget*>>& pages);
    void restoreDynamicTab(const QString& pageName);
    QString currentDynamicTabName() const;

    void installShortcuts(QWidget* shortcutParent);

private:
    void rebuildMenu();
    void openMoreMenu(bool selectLastItem);
    void switchDynamicTab(QWidget* page, const QString& title);

    QTabWidget* m_tabWidget = nullptr;
    QMenu* m_moreMenu = nullptr;
    QVector<QPair<QString, QWidget*>> m_extraPages;
    int m_previousTab = 0;
    bool m_menuJustClosed = false;
};


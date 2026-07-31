#include "tabcoordinator.h"

#include <QMenu>
#include <QShortcut>
#include <QTabBar>
#include <QTabWidget>
#include <QTimer>

TabCoordinator::TabCoordinator(QTabWidget* tabWidget, QObject* parent)
    : QObject(parent), m_tabWidget(tabWidget), m_moreMenu(new QMenu(tabWidget)) {

    connect(m_moreMenu, &QMenu::aboutToHide, this, [this]() {
        m_menuJustClosed = true;
        QTimer::singleShot(200, this, [this]() {
            m_menuJustClosed = false;
        });
    });

    connect(m_tabWidget, &QTabWidget::tabBarClicked, this, [this](int index) {
        if (index == 3) {
            // Second click on the open menu button should only close the menu.
            if (m_moreMenu->isVisible()) {
                m_moreMenu->hide();
                return;
            }
            if (!m_menuJustClosed)
                openMoreMenu(false);
        } else {
            m_previousTab = index;
        }
    });

    connect(m_tabWidget, &QTabWidget::currentChanged, this, [this](int index) {
        if (index == 3) {
            m_tabWidget->setCurrentIndex(m_previousTab);
            return;
        }
        m_previousTab = index;
    });
}

void TabCoordinator::setExtraPages(const QVector<QPair<QString, QWidget*>>& pages) {
    m_extraPages = pages;
    rebuildMenu();
}

void TabCoordinator::restoreDynamicTab(const QString& pageName) {
    for (const auto& [name, page] : m_extraPages) {
        if (name == pageName) {
            switchDynamicTab(page, name);
            return;
        }
    }
}

QString TabCoordinator::currentDynamicTabName() const {
    const QWidget* dynWidget = m_tabWidget->widget(2);
    for (const auto& [name, page] : m_extraPages) {
        if (page == dynWidget)
            return name;
    }
    return QString();
}

void TabCoordinator::installShortcuts(QWidget* shortcutParent) {
    auto* prevTab = new QShortcut(QKeySequence(Qt::AltModifier | Qt::Key_Left), shortcutParent);
    connect(prevTab, &QShortcut::activated, this, [this]() {
        const int current = m_tabWidget->currentIndex();
        const int next = (current - 1 + 3) % 3;
        m_tabWidget->setCurrentIndex(next);
        if (QWidget* w = m_tabWidget->currentWidget())
            w->setFocus();
    });

    auto* nextTab = new QShortcut(QKeySequence(Qt::AltModifier | Qt::Key_Right), shortcutParent);
    connect(nextTab, &QShortcut::activated, this, [this]() {
        const int current = m_tabWidget->currentIndex();
        const int next = (current + 1) % 3;
        m_tabWidget->setCurrentIndex(next);
        if (QWidget* w = m_tabWidget->currentWidget())
            w->setFocus();
    });

    auto* openDown = new QShortcut(QKeySequence(Qt::AltModifier | Qt::Key_Down), shortcutParent);
    connect(openDown, &QShortcut::activated, this, [this]() {
        openMoreMenu(false);
    });

    auto* openUp = new QShortcut(QKeySequence(Qt::AltModifier | Qt::Key_Up), shortcutParent);
    connect(openUp, &QShortcut::activated, this, [this]() {
        openMoreMenu(true);
    });
}

void TabCoordinator::rebuildMenu() {
    m_moreMenu->clear();
    for (const auto& [name, page] : m_extraPages) {
        m_moreMenu->addAction(name, this, [this, page, name]() {
            switchDynamicTab(page, name);
            m_previousTab = 2;
        });
    }
}

void TabCoordinator::openMoreMenu(bool selectLastItem) {
    if (m_moreMenu->isVisible())
        return;

    const QRect tabRect = m_tabWidget->tabBar()->tabRect(3);
    const QPoint pos = m_tabWidget->tabBar()->mapToGlobal(tabRect.bottomLeft());
    m_moreMenu->popup(pos);
    m_moreMenu->setFocus();

    if (m_moreMenu->actions().isEmpty())
        return;

    if (selectLastItem)
        m_moreMenu->setActiveAction(m_moreMenu->actions().last());
    else
        m_moreMenu->setActiveAction(m_moreMenu->actions().first());
}

void TabCoordinator::switchDynamicTab(QWidget* page, const QString& title) {
    QWidget* current = m_tabWidget->widget(2);
    if (current == page) {
        m_tabWidget->setCurrentIndex(2);
        return;
    }

    m_tabWidget->removeTab(2);
    m_tabWidget->insertTab(2, page, title);
    m_tabWidget->setCurrentIndex(2);
}



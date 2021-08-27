﻿#include "routingcontext.h"
#include "mainwindow.h"

#include "data/train/routing.h"
#include "editors/routing/parseroutingdialog.h"

#include <QLabel>
#include <QLineEdit>

RoutingContext::RoutingContext(Diagram &diagram, SARibbonContextCategory *context,
                               MainWindow *mw_):
    QObject(mw_), _diagram(diagram), cont(context), mw(mw_)
{
    initUI();
}

void RoutingContext::initUI()
{
    auto* page=cont->addCategoryPage(tr("交路编辑"));

    auto* panel=page->addPannel(tr("基本信息"));

    auto* w = new QWidget;
    auto* vlay = new QVBoxLayout(w);

    auto* lab = new QLabel(tr("当前交路"));
    vlay->addWidget(lab);
    lab->setAlignment(Qt::AlignCenter);

    edName = new QLineEdit;
    edName->setFocusPolicy(Qt::NoFocus);
    edName->setAlignment(Qt::AlignCenter);
    edName->setFixedWidth(150);
    vlay->addWidget(edName);
    panel->addWidget(w, SARibbonPannelItem::Large);

    auto* act = new QAction(QIcon(":/icons/trainline.png"), tr("高亮显示"), this);
    act->setCheckable(true);
    connect(act, &QAction::triggered, this, &RoutingContext::toggleHighlight);
    auto* btn = panel->addLargeAction(act);
    btn->setMinimumWidth(80);
    btnHighlight = btn;

    panel=page->addPannel(tr("编辑"));
    act=new QAction(QIcon(":/icons/edit.png"),tr("交路编辑"),this);
    connect(act,SIGNAL(triggered()),this,SLOT(actEdit()));
    btn=panel->addLargeAction(act);
    btn->setMinimumWidth(80);

    panel = page->addPannel(tr("工具"));
    act = new QAction(QIcon(":/icons/text.png"), tr("文本解析"), this);
    act->setToolTip(tr("单交路文本解析\n"
        "输入车次套用的文本，从中解析交路序列，并直接应用于当前交路。"));
    connect(act, &QAction::triggered, this, &RoutingContext::actParseText);
    panel->addMediumAction(act);
}

int RoutingContext::getRoutingWidgetIndex(std::shared_ptr<Routing> routing)
{
    for(int i=0;i<routingEdits.size();i++){
        auto p=routingEdits.at(i);
        if(p->getRouting() == routing)
            return i;
    }
    return -1;
}

int RoutingContext::getRoutingWidgetIndex(RoutingEdit* w)
{
    for (int i = 0; i < routingEdits.size(); i++) {
        auto p = routingEdits.at(i);
        if (p == w)
            return i;
    }
    return -1;
}

void RoutingContext::updateRoutingEditBasic(std::shared_ptr<Routing> routing)
{
    for (int i = 0; i < routingEdits.size(); i++) {
        auto p = routingEdits.at(i);
        if (p->getRouting() == routing) {
            p->refreshBasicData();
            routingDocks.at(i)->setWindowTitle(tr("交路编辑 - %1").arg(routing->name()));
        }
    }
}

void RoutingContext::updateRoutingEdit(std::shared_ptr<Routing> routing)
{
    for (int i = 0; i < routingEdits.size(); i++) {
        auto p = routingEdits.at(i);
        if (p->getRouting() == routing) {
            p->refreshData();
            routingDocks.at(i)->setWindowTitle(tr("交路编辑 - %1").arg(routing->name()));
        }
    }
}

void RoutingContext::actEdit()
{
    openRoutingEditWidget(_routing);
}

void RoutingContext::refreshData()
{
    if (_routing) {
        edName->setText(_routing->name());
    }
}

void RoutingContext::toggleHighlight(bool on)
{
    if (on != _routing->isHighlighted()) {
        onRoutingHighlightChanged(_routing, on);
        emit routingHighlightChanged(_routing);
    }
}

void RoutingContext::removeRoutingEdit()
{
    auto* w = static_cast<RoutingEdit*>(sender());
    int i = getRoutingWidgetIndex(w);
    if (i >= 0) {
        auto* dock = routingDocks.takeAt(i);
        routingEdits.removeAt(i);
        dock->deleteDockWidget();
    }
}

void RoutingContext::actParseText()
{
    auto* dialog = new ParseRoutingDialog(_diagram.trainCollection(), true,
        _routing, mw);
    connect(dialog, &ParseRoutingDialog::routingParsed, this,
        &RoutingContext::actRoutingOrderChange);
    dialog->show();
}

void RoutingContext::refreshAllData()
{
    refreshData();
}

void RoutingContext::setRouting(std::shared_ptr<Routing> routing)
{
    _routing=routing;
    refreshData();
}

void RoutingContext::openRoutingEditWidget(std::shared_ptr<Routing> routing)
{
    int i = getRoutingWidgetIndex(routing);
    if (i == -1) {
        auto* w = new RoutingEdit(_diagram.trainCollection(), routing);
        auto* dock = new ads::CDockWidget(tr("交路编辑 - %1").arg(routing->name()));
        dock->setWidget(w);
        connect(w, &RoutingEdit::closeDock, this, &RoutingContext::removeRoutingEdit);
        routingEdits.push_back(w);
        routingDocks.push_back(dock);
        mw->getManager()->addDockWidgetFloating(dock);
        mw->getRoutingMenu()->addAction(dock->toggleViewAction());

        connect(w, &RoutingEdit::routingInfoChanged, this,
            &RoutingContext::actRoutingInfoChange);
        connect(w, &RoutingEdit::routingOrderChanged, this,
            &RoutingContext::actRoutingOrderChange);
        connect(w, &RoutingEdit::focusInRouting,
            mw, &MainWindow::focusInRouting);
    }
    else {
        auto dock = routingDocks.at(i);
        if (dock->isClosed()) {
            dock->toggleView(true);
        }
        else {
            dock->setAsCurrentTab();
        }
    }
}

void RoutingContext::removeRoutingEditWidget(std::shared_ptr<Routing> routing)
{
    for (int i = 0; i < routingEdits.size();) {
        if (routingEdits.at(i)->getRouting() == routing)
            removeRoutingEditWidgetAt(i);
        else i++;
    }
}

void RoutingContext::removeRoutingEditWidgetAt(int i)
{
    auto* dock = routingDocks.takeAt(i);
    routingEdits.removeAt(i);
    dock->deleteDockWidget();
}

void RoutingContext::onRoutingHighlightChanged(std::shared_ptr<Routing> routing, bool on)
{
    routing->setHighlight(on);
    mw->setRoutingHighlight(routing, on);
    if (routing == _routing) {
        btnHighlight->setChecked(on);
    }
}

void RoutingContext::actRoutingInfoChange(std::shared_ptr<Routing> routing, 
    std::shared_ptr<Routing> info)
{
    mw->getUndoStack()->push(new qecmd::ChangeRoutingInfo(routing, info, this));
}

void RoutingContext::commitRoutingInfoChange(std::shared_ptr<Routing> routing)
{
    if (routing == _routing) {
        refreshData();
    }
    updateRoutingEditBasic(routing);
    emit routingInfoChanged(routing);
}

void RoutingContext::actRoutingOrderChange(std::shared_ptr<Routing> routing, 
    std::shared_ptr<Routing> info)
{
    mw->getUndoStack()->push(new qecmd::ChangeRoutingOrder(routing, info, this));
}

void RoutingContext::commitRoutingOrderChange(std::shared_ptr<Routing> routing,
    QSet<std::shared_ptr<Train>> takenTrains)
{
    for (const auto& p : routing->order()) {
        if (!p.isVirtual()) {
            mw->repaintTrainLines(p.train());
        }
    }
    foreach(auto p, takenTrains) {
        mw->repaintTrainLines(p);
    }

    if (routing == _routing) {
        refreshData();
    }
    updateRoutingEdit(routing);
    emit routingInfoChanged(routing);
}

void RoutingContext::onRoutingRemoved(std::shared_ptr<Routing> routing)
{
    auto s = routing->trainSet();
    if (!s.empty()) {
        mw->repaintTrainLines(s);
    }
    removeRoutingEditWidget(routing);
    if (routing == _routing) {
        _routing.reset();
        mw->focusOutRouting();
    }
}


qecmd::ChangeRoutingInfo::ChangeRoutingInfo(std::shared_ptr<Routing> routing_,
    std::shared_ptr<Routing> info_,
    RoutingContext* context, QUndoCommand* parent) :
    QUndoCommand(QObject::tr("更新标尺信息: %1").arg(info_->name()), parent),
    routing(routing_), info(info_), cont(context) {}

void qecmd::ChangeRoutingInfo::redo()
{
    routing->swapBase(*info);
    cont->commitRoutingInfoChange(routing);
}

void qecmd::ChangeRoutingInfo::undo()
{
    routing->swapBase(*info);
    cont->commitRoutingInfoChange(routing);
}

qecmd::ChangeRoutingOrder::ChangeRoutingOrder(std::shared_ptr<Routing> routing_, 
    std::shared_ptr<Routing> info_, RoutingContext* context, QUndoCommand* parent):
    QUndoCommand(QObject::tr("更新交路序列: %1").arg(info_->name()),parent),
    routing(routing_),data(info_),cont(context)
{
}

void qecmd::ChangeRoutingOrder::undo()
{
    commit();
}

void qecmd::ChangeRoutingOrder::redo()
{
    commit();
}

void qecmd::ChangeRoutingOrder::commit()
{
    routing->swap(*data);
    routing->updateTrainHooks();
    auto s = routing->takenTrains(*data);
    foreach(auto & p, s) {
        p->resetRoutingSimple();
    }
    cont->commitRoutingOrderChange(routing, std::move(s));
}

﻿#ifndef QETRC_MOBILE_2
#include "configdialog.h"
#include "util/buttongroup.hpp"

#include "mainwindow/viewcategory.h"
#include "mainwindow/mainwindow.h"
#include "data/diagram/diagrampage.h"

#include <QCheckBox>
#include <QColorDialog>
#include <QComboBox>
#include <QDoubleSpinBox>
#include <QFormLayout>
#include <QGroupBox>
#include <QMessageBox>


ConfigDialog::ConfigDialog(Config &cfg,bool forDefault_, QWidget *parent):
    QDialog(parent),_cfg(cfg),forDefault(forDefault_),page(nullptr)
{
    setAttribute(Qt::WA_DeleteOnClose);
    if (forDefault) {
        setWindowTitle(tr("系统默认显示设置"));
    }
    else
        setWindowTitle(tr("运行图显示设置"));
    initUI();
    refreshData();
}

ConfigDialog::ConfigDialog(Config& cfg, const std::shared_ptr<DiagramPage>& page, QWidget* parent):
    QDialog(parent),_cfg(cfg),forDefault(false),page(page)
{
    setAttribute(Qt::WA_DeleteOnClose);
    setWindowTitle(tr("运行图显示设置 - %1").arg(page->name()));
    initUI();
    refreshData();
}

void ConfigDialog::initUI()
{
    resize(600, 700);
    auto* hlay=new QHBoxLayout;
    QVBoxLayout* vlay;
    QFormLayout* form;
    QGroupBox* gb;

    QSpinBox* sp;
    QDoubleSpinBox* sd;

    vlay=new QVBoxLayout;
    // 1. 时间轴
    if constexpr (true){
        gb=new QGroupBox(tr("时间轴"));
        form=new QFormLayout;

        sp=new QSpinBox;
        sp->setRange(0,24);
        spStartHour=sp;
        sp->setToolTip(tr("运行图起始的时刻，整点小时"));
        form->addRow(tr("起始时刻"),sp);

        sp=new QSpinBox;
        sp->setRange(0,24);
        sp->setToolTip(tr("运行图结束的时刻，整点小时"));
        form->addRow(tr("结束时刻"),sp);
        spEndHour=sp;

        sd=new QDoubleSpinBox;
        sd->setRange(0,10000000);
        sd->setToolTip(tr("时间轴上每个像素对应的秒数。数值越大，显示横轴比例越小"));
        form->addRow(tr("横轴每像素秒数"),sd);
        sdScaleX=sd;

        //sp=new QSpinBox;
        //sp->setRange(0,59);
        //sp->setToolTip(tr("每小时的时长范围内纵线的数目，不包括整点线"));
        //form->addRow(tr("每小时纵线数"),sp);
        //spVLines=sp;

        cbVLineStyle = new QComboBox;
        cbVLineStyle->addItem(tr("第二级细实线，第三级细虚线"));
        cbVLineStyle->addItem(tr("第二级细虚线，第三级细实线"));
        form->addRow(tr("纵线形式"), cbVLineStyle);

        sd = new QDoubleSpinBox;
        sd->setRange(0, 60);
        sd->setToolTip(tr("小时线以下第一级纵线（粗实线）的间隔分钟数"));
        sd->setSuffix(tr(" 分钟 (min)"));
        form->addRow(tr("一级纵线间隔"), sd);
        sdVLineBold = sd;

        sd = new QDoubleSpinBox;
        sd->setRange(0, 60);
        sd->setToolTip(tr("小时线以下第二级纵线（细实线或细虚线）的间隔分钟数"));
        sd->setSuffix(tr(" 分钟 (min)"));
        form->addRow(tr("二级纵线间隔"), sd);
        sdVLineSecond = sd;

        sd = new QDoubleSpinBox;
        sd->setRange(0, 60);
        sd->setToolTip(tr("小时线以下第三级纵线（细虚线或细实线）的间隔分钟数"));
        sd->setSuffix(tr(" 分钟 (min)"));
        form->addRow(tr("三级纵线间隔"), sd);
        sdVLineThird = sd;

        sp=new QSpinBox;
        sp->setRange(0,10000000);
        sp->setToolTip(tr("在横轴上标记分钟数标签的最小间隔（像素）"));
        form->addRow(tr("分钟标记间隔"),sp);
        spMinMarkInter=sp;

        gb->setLayout(form);

        vlay->addWidget(gb);
    }

    // 2. 边距控制
    if constexpr(true){
        gb=new QGroupBox(tr("运行图边距"));
        form=new QFormLayout;

        sp=new QSpinBox;
        sp->setToolTip(tr("运行图顶端至第一个车站的距离"));
        sp->setRange(0,10000);
        sp->setSingleStep(5);
        form->addRow(tr("上边距"),sp);
        spMarginUp=sp;

        sp=new QSpinBox;
        sp->setToolTip(tr("运行图底端至最后一个车站水平线的距离"));
        sp->setRange(0,10000);
        sp->setSingleStep(5);
        form->addRow(tr("下边距"),sp);
        spMarginDown=sp;

        sp=new QSpinBox;
        sp->setToolTip(tr("运行图左侧排图标尺或区间里程栏的宽度"));
        sp->setRange(0,10000);
        sp->setSingleStep(5);
        form->addRow(tr("排图标尺栏宽度"),sp);
        spRulerWidth=sp;

        sp=new QSpinBox;
        sp->setToolTip(tr("运行图左侧延长公里栏宽度"));
        sp->setRange(0,10000);
        sp->setSingleStep(5);
        form->addRow(tr("延长公里栏宽度"),sp);
        spMileWidth=sp;

        sp=new QSpinBox;
        sp->setToolTip(tr("运行图左侧显示站名栏的宽度"));
        sp->setRange(0,10000);
        sp->setSingleStep(5);
        form->addRow(tr("站名栏宽度"),sp);
        spStationWidth=sp;

        sp = new QSpinBox;
        sp->setToolTip(tr("运行图右侧显示客货断面对数栏的宽度"));
        sp->setRange(0, 10000);
        sp->setSingleStep(5);
        form->addRow(tr("断面对数栏宽度"), sp);
        spCountWidth = sp;

        sp=new QSpinBox;
        sp->setToolTip(tr("运行图左右边界值站名栏边界的宽度"));
        sp->setRange(0,10000);
        sp->setSingleStep(5);
        form->addRow(tr("左右空白宽度"),sp);
        spHWhite=sp;

        sp=new QSpinBox;
        sp->setToolTip(tr("同一张运行图上，多条线路之间，前一线路末站和后一线路首站之间的距离"));
        sp->setRange(0,100000);
        sp->setSingleStep(5);
        form->addRow(tr("基线间距"),sp);
        spInterval=sp;

        gb->setLayout(form);
        vlay->addWidget(gb);
    }

    // 4. 格线控制
    if constexpr (true) {
        gb = new QGroupBox(tr("格线控制"));
        form = new QFormLayout;

        sd = new QDoubleSpinBox;
        sd->setRange(0, 100);
        sd->setSingleStep(0.5);
        sd->setToolTip(tr("设置水平和垂直细格线的宽度"));
        sdSlimWidth = sd;
        form->addRow(tr("细格线宽度"), sd);

        sd = new QDoubleSpinBox;
        sd->setRange(0, 100);
        sd->setSingleStep(0.5);
        sd->setToolTip(tr("设置水平和垂直粗格线宽度"));
        sdBoldWidth = sd;
        form->addRow(tr("粗格线宽度"), sd);

        auto* btn = new QPushButton;
        btnGridColor = btn;
        connect(btn, &QPushButton::clicked, this, &ConfigDialog::actGridColor);
        form->addRow(tr("格线颜色"), btn);

        btn = new QPushButton;
        btnTextColor = btn;
        connect(btn, &QPushButton::clicked, this, &ConfigDialog::actTextColor);
        form->addRow(tr("字体颜色"), btn);

        gb->setLayout(form);
        vlay->addWidget(gb);
    }

    hlay->addLayout(vlay);
    vlay=new QVBoxLayout;

    // 3. 空间轴
    if constexpr (true){
        gb=new QGroupBox(tr("空间（距离/车站）轴"));
        form=new QFormLayout;

        sd=new QDoubleSpinBox;
        sd->setRange(0,10000);
        sd->setToolTip(tr("按里程排图时有效，每公里对应的像素数。数值越大，纵轴比例越大"));
        form->addRow(tr("纵轴每公里像素数"),sd);
        sdScaleYdist=sd;

        sd=new QDoubleSpinBox;
        sd->setRange(0,10000);
        sd->setToolTip(tr("按标尺排图时有效，每个像素对应排图标尺中通通时分的秒数。"
            "数值越大，纵轴比例越小"));
        form->addRow(tr("纵轴每秒像素数"),sd);
        sdScaleYsec=sd;

        sp = new QSpinBox;
        sp->setRange(0, 100);
        sp->setToolTip(tr("最低显示车站等级，低于此等级（数值更大）的车站水平线将不显示。"));
        form->addRow(tr("最低显示等级"), sp);
        spShowLevel = sp;

        sp = new QSpinBox;
        sp->setRange(0, 100);
        sp->setToolTip(tr("最低粗线显示等级，高于（数值小于，包含）此等级的车站将显示为粗线。"));
        form->addRow(tr("最低粗线等级"), sp);
        spBoldLevel = sp;

        auto* ck = new QCheckBox(tr("显示"));
        ckShowRuler = ck;
        ck->setToolTip(tr("控制运行图左侧的[排图标尺]或者[区间距离]栏是否显示"));
        form->addRow(tr("排图标尺栏"), ck);

        ck = new QCheckBox(tr("显示"));
        ckShowMile = ck;
        ck->setToolTip(tr("控制运行图左侧的[延长公里]栏是否显示"));
        form->addRow(tr("延长公里栏"), ck);

        ck = new QCheckBox(tr("显示"));
        ckShowCount = ck;
        ck->setToolTip(tr("控制运行图右侧的[客货对数]栏是否显示"));
        form->addRow(tr("客货对数栏"), ck);

        gb->setLayout(form);
        vlay->addWidget(gb);
    }



    // 5. 运行线
    if constexpr (true){
        gb=new QGroupBox(tr("运行线控制"));
        form=new QFormLayout;

        sp=new QSpinBox;
        sp->setRange(1,10000);
        sp->setToolTip(tr("当设置值大于1时，点击列车运行线周边所设定数值倍数"
            "的范围时，仍然可以选中运行线。请注意此时可能增加运行图铺画代价。" ));
        form->addRow(tr("有效选择宽度"),sp);
        spValidWidth=sp;

        auto* cb=new QComboBox;
        cb->addItems({tr("不显示"),tr("仅选中车次显示"),tr("全部显示")});
        cb->setToolTip(tr("是否在运行线侧显示图定时刻的分钟个位数。此功能可能带来一定的效率问题。"));
        form->addRow(tr("图中显示时刻"),cb);
        cbShowTimeMark=cb;

        auto* ck=new QCheckBox;
        ck->setToolTip(tr("是否在运行线结束标签显示车次"));
        form->addRow(tr("结束标签车次"),ck);
        ckEndLabel=ck;

        ck=new QCheckBox;
        ck->setToolTip(tr("如果选中，则总是在标签上显示完整车次；"
                "否则优先显示方向车次，如果方向车次为空则显示完整车次。"));
        form->addRow(tr("显示完整车次"),ck);
        ckFullName=ck;

        gb->setLayout(form);
        vlay->addWidget(gb);
    }

    // 6. 标签控制
    if constexpr (true){
        gb=new QGroupBox(tr("车次标签高度控制"));
        form=new QFormLayout;

        auto* ck=new QCheckBox(tr("启用"));
        ck->setToolTip(tr("若启用，则在同站、同方向的车次标签启用冲突检测，自动设置高度"
            "以回避冲突；如不启用，则分别按照开始标签和结束标签高度设定。"));
        ckAvoidCollid=ck;
        form->addRow(tr("标签自动偏移"),ck);
        connect(ck, SIGNAL(toggled(bool)),
                this,SLOT(onAvoidCollidChanged(bool)));

        sp=new QSpinBox;
        sp->setRange(0,10000);
        form->addRow(tr("起始标签高度"),sp);
        spStartLabelHeight=sp;

        sp=new QSpinBox;
        sp->setRange(0,10000);
        form->addRow(tr("结束标签高度"),sp);
        spEndLabelHeight=sp;

        sp=new QSpinBox;
        sp->setRange(0,10000);
        form->addRow(tr("基准标签高度"),sp);
        spBaseHeight=sp;

        sp=new QSpinBox;
        sp->setRange(0,10000);
        form->addRow(tr("标签层级高度"),sp);
        spStepHeight=sp;

        gb->setLayout(form);
        vlay->addWidget(gb);
    }

    hlay->addLayout(vlay);

    auto* scroll=new QScrollArea;
    auto* w = new QWidget;
    w->setLayout(hlay);
    scroll->setWidget(w);
    scroll->setWidgetResizable(true);

    auto* tlay=new QVBoxLayout;   //top Layout
    tlay->addWidget(scroll);

    auto* g=new ButtonGroup<4>({"确定","应用","还原","取消"});
    g->connectAll(SIGNAL(clicked()),this,
                  {SLOT(actOk()), SLOT(actApply()),SLOT(refreshData()), SLOT(close())});
    tlay->addLayout(g);

    setLayout(tlay);

}

#define SET_VALUE(_spin,_key) _spin->setValue(_cfg._key)
#define SET_MARGIN(_spin,_key) _spin->setValue(_cfg.margins._key)
#define SET_CHECK(_check,_key) _check->setChecked(_cfg._key)

void ConfigDialog::refreshData()
{
    //时间轴
    SET_VALUE(spStartHour, start_hour);
    SET_VALUE(spEndHour, end_hour);
    SET_VALUE(sdScaleX, seconds_per_pix);
    SET_VALUE(sdVLineBold, minutes_per_vertical_bold);
    SET_VALUE(sdVLineSecond, minutes_per_vertical_second);
    SET_VALUE(sdVLineThird, minutes_per_vertical_line);
    SET_VALUE(spMinMarkInter, minute_mark_gap_pix);
    cbVLineStyle->setCurrentIndex(_cfg.dash_as_second_level_vline ? 1 : 0);

    //运行图边距
    SET_MARGIN(spMarginUp, up);
    SET_MARGIN(spMarginDown, down);
    SET_MARGIN(spRulerWidth,ruler_label_width);
    SET_MARGIN(spMileWidth, mile_label_width);
    SET_MARGIN(spStationWidth, label_width);
    SET_MARGIN(spCountWidth, count_label_width);
    spHWhite->setValue(_cfg.margins.right - _cfg.margins.right_white - _cfg.margins.label_width
        - _cfg.margins.count_label_width);
    SET_MARGIN(spInterval, gap_between_railways);

    //格线控制
    SET_VALUE(sdSlimWidth, default_grid_width);
    SET_VALUE(sdBoldWidth, bold_grid_width);

    //空间轴
    SET_VALUE(sdScaleYdist, pixels_per_km);
    SET_VALUE(sdScaleYsec, seconds_per_pix_y);
    SET_VALUE(spShowLevel, show_station_level);
    SET_VALUE(spBoldLevel, bold_line_level);
    SET_CHECK(ckShowRuler, show_ruler_bar);
    SET_CHECK(ckShowMile, show_mile_bar);
    SET_CHECK(ckShowCount, show_count_bar);

    //运行线控制
    SET_VALUE(spValidWidth, valid_width);
    cbShowTimeMark->setCurrentIndex(_cfg.show_time_mark);
    ckEndLabel->setChecked(_cfg.end_label_name);
    ckFullName->setChecked(_cfg.show_full_train_name);

    //标签高度
    ckAvoidCollid->setChecked(_cfg.avoid_cover);
    SET_VALUE(spStartLabelHeight, start_label_height);
    SET_VALUE(spEndLabelHeight, end_label_height);
    SET_VALUE(spBaseHeight, base_label_height);
    SET_VALUE(spStepHeight, step_label_height);

    gridColor = _cfg.grid_color;
    textColor = _cfg.text_color;
    btnGridColor->setText(gridColor.name().toUpper());
    btnGridColor->setStyleSheet(QStringLiteral(
        "QPushButton { background-color: rgb(%1, %2, %3); }").arg(gridColor.red())
        .arg(gridColor.green()).arg(gridColor.blue()));
    btnTextColor->setText(gridColor.name().toUpper());
    btnTextColor->setStyleSheet(QStringLiteral(
        "QPushButton { background-color: rgb(%1, %2, %3); }").arg(textColor.red())
        .arg(textColor.green()).arg(textColor.blue()));
}

#undef SET_VALUE
#undef SET_MARGIN

#define GET_VALUE(_spin,_key) cnew._key=_spin->value()
#define GET_MARGIN(_spin,_key) cnew.margins._key=_spin->value()
#define GET_CHECK(_check,_key) cnew._key=_check->isChecked()

void ConfigDialog::actApply()
{
    //暂时：一律重新铺画！
    Config cnew;
    bool repaint = false;

    //时间轴
    GET_VALUE(spStartHour, start_hour);
    GET_VALUE(spEndHour, end_hour);
    GET_VALUE(sdScaleX, seconds_per_pix);
    GET_VALUE(sdVLineBold, minutes_per_vertical_bold);
    GET_VALUE(sdVLineSecond, minutes_per_vertical_second);
    GET_VALUE(sdVLineThird, minutes_per_vertical_line);
    GET_VALUE(spMinMarkInter, minute_mark_gap_pix);
    cnew.dash_as_second_level_vline = bool(cbVLineStyle->currentIndex());

    //运行图边距
    GET_MARGIN(spMarginUp, up);
    GET_MARGIN(spMarginDown, down);
    GET_MARGIN(spRulerWidth, ruler_label_width);
    GET_MARGIN(spMileWidth, mile_label_width);
    GET_MARGIN(spStationWidth, label_width);
    GET_MARGIN(spCountWidth, count_label_width);
    cnew.margins.left = spHWhite->value() + cnew.margins.ruler_label_width +
        cnew.margins.mile_label_width + cnew.margins.label_width + cnew.margins.left_white;
    cnew.margins.right = spHWhite->value() + cnew.margins.right_white +
        cnew.margins.label_width + cnew.margins.count_label_width;
    GET_MARGIN(spInterval, gap_between_railways);

    //格线控制
    GET_VALUE(sdSlimWidth, default_grid_width);
    GET_VALUE(sdBoldWidth, bold_grid_width);

    //空间轴
    GET_VALUE(sdScaleYdist, pixels_per_km);
    GET_VALUE(sdScaleYsec, seconds_per_pix_y);
    GET_VALUE(spShowLevel, show_station_level);
    GET_VALUE(spBoldLevel, bold_line_level);
    GET_CHECK(ckShowRuler, show_ruler_bar);
    GET_CHECK(ckShowMile, show_mile_bar);
    GET_CHECK(ckShowCount, show_count_bar);

    //运行线控制
    GET_VALUE(spValidWidth, valid_width);
    cnew.show_time_mark = cbShowTimeMark->currentIndex();
    cnew.end_label_name = ckEndLabel->isChecked();
    cnew.show_full_train_name = ckFullName->isChecked();

    //标签高度
    cnew.avoid_cover = ckAvoidCollid->isChecked();
    GET_VALUE(spStartLabelHeight, start_label_height);
    GET_VALUE(spEndLabelHeight, end_label_height);
    GET_VALUE(spBaseHeight, base_label_height);
    GET_VALUE(spStepHeight, step_label_height);
    
    cnew.grid_color = gridColor;
    cnew.text_color = textColor;

    // check and warnings
    QString warns{};
    if (std::fmod(60., cnew.minutes_per_vertical_line) > 1e-6 ||
        std::fmod(60., cnew.minutes_per_vertical_bold) > 1e-6 ||
        std::fmod(60., cnew.minutes_per_vertical_second) > 1e-6) {
        warns.append(tr("[一级|二级|三级]纵线间隔值不是60的因数，可能导致铺画异常\n"));
    }
    if (std::fmod(cnew.minutes_per_vertical_bold, cnew.minutes_per_vertical_second) > 1e-6 ||
        std::fmod(cnew.minutes_per_vertical_second, cnew.minutes_per_vertical_line) > 1e-6) {
        warns.append(tr("一级纵线间隔不是二级纵线间隔的整数倍，或二级纵线间隔不是三级纵线间隔的整数倍，"
            "可能导致铺画异常\n"));
    }

    if (!warns.isEmpty()) {
        warns.append(tr("以上警告不影响结果提交，但强烈建议修正以避免可能的异常行为！"));
        QMessageBox::warning(this, tr("警告"), warns);
    }

    if (page) {
        emit onPageConfigApplied(_cfg, cnew, repaint, page);
    }
    else {
        emit onConfigApplied(_cfg, cnew, repaint && !forDefault, forDefault);
    }
}

void ConfigDialog::actOk()
{
    actApply();
    done(QDialog::Accepted);
}

void ConfigDialog::onAvoidCollidChanged(bool on)
{
    spStartLabelHeight->setEnabled(!on);
    spEndLabelHeight->setEnabled(!on);
    spBaseHeight->setEnabled(on);
    spStepHeight->setEnabled(on);
}

void ConfigDialog::actGridColor()
{
    auto color = QColorDialog::getColor(gridColor, this, tr("格线颜色"));
    if (color.isValid()) {
        gridColor = color;
        btnGridColor->setText(color.name());
        btnGridColor->setText(gridColor.name().toUpper());
        btnGridColor->setStyleSheet(QStringLiteral(
            "QPushButton { background-color: rgb(%1, %2, %3); }").arg(gridColor.red())
            .arg(gridColor.green()).arg(gridColor.blue()));
    }
}

void ConfigDialog::actTextColor()
{
    auto color = QColorDialog::getColor(textColor, this, tr("字体颜色"));
    if (color.isValid()) {
        textColor = color;
        btnTextColor->setText(color.name());
        btnTextColor->setText(gridColor.name().toUpper());
        btnTextColor->setStyleSheet(QStringLiteral(
            "QPushButton { background-color: rgb(%1, %2, %3); }").arg(textColor.red())
            .arg(textColor.green()).arg(textColor.blue()));
    }
}

qecmd::ChangeConfig::ChangeConfig(Config& cfg_, const Config& newcfg_, 
    bool repaint_, bool forDefault_, ViewCategory* cat_, QUndoCommand* parent):
    QUndoCommand(parent), cfg(cfg_), newcfg(newcfg_),
    repaint(repaint_), forDefault(forDefault_),
    cat(cat_)
{
    if (forDefault) {
        setText(QObject::tr("更改系统默认显示设置"));
    }
    else {
        setText(QObject::tr("更改运行图显示设置"));
    }
}

void qecmd::ChangeConfig::undo()
{
    std::swap(cfg, newcfg);
    cat->commitConfigChange(cfg, repaint);
    if (forDefault)
        cat->saveDefaultConfigs();
}

void qecmd::ChangeConfig::redo()
{
    std::swap(cfg, newcfg);
    cat->commitConfigChange(cfg, repaint);
    if (forDefault)
        cat->saveDefaultConfigs();
}

void qecmd::ChangePassedStation::undo()
{
    mw->commitPassedStationChange(valueold);
}

void qecmd::ChangePassedStation::redo()
{
    mw->commitPassedStationChange(valuenew);
}

qecmd::ChangePageConfig::ChangePageConfig(Config& cfg_, const Config& newcfg_, bool repaint_,
    std::shared_ptr<DiagramPage> page, ViewCategory* cat_, QUndoCommand* parent):
    ChangeConfig(cfg_,newcfg_,repaint_,false,cat_,parent),page(page)
{
    setText(QObject::tr("更改运行图页面设置: %1").arg(page->name()));
}

void qecmd::ChangePageConfig::undo()
{
    std::swap(cfg, newcfg);
    cat->commitPageConfigChange(page, repaint);
}
void qecmd::ChangePageConfig::redo()
{
    std::swap(cfg, newcfg);
    cat->commitPageConfigChange(page, repaint);
}

qecmd::ChangePageScale::ChangePageScale(Config& cfg_, const Config& newcfg_,
    bool repaint_, std::shared_ptr<DiagramPage> page, ViewCategory* cat_, QUndoCommand* parent):
    ChangePageConfig(cfg_,newcfg_,repaint_,page,cat_,parent)
{
    setText(QObject::tr("更改运行图显示比例: %1").arg(page->name()));
}

bool qecmd::ChangePageScale::mergeWith(const QUndoCommand* cmd)
{
    if (id() != cmd->id()) {
        return false;
    }
    auto* another = static_cast<const qecmd::ChangePageScale*>(cmd);
    return page == another->page;
}
#endif

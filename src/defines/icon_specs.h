﻿/**
 * 2024.03.26  
 * This file provides macro definitions for the icons used in this program.
 * The file should only be included in related cpp files.
 * Here, QE stands for qETRC, ICN stands for icon. 
 */

#pragma once

#include <QApplication>
#include <QStyle>
#include <QIcon>

/**
 * For the icons in QStyle::standardIcon. 
 * The argument _name starts from SP_
 */
#define QE_STD_ICON(_name) qApp->style()->standardIcon(QStyle::SP_##_name)

#define QE_RSC_ICON_PREFIX ":/icons/"

/**
 * The icon using the files provided in rsc.
 */
#define QE_RSC_ICON(_path) QIcon(QE_RSC_ICON_PREFIX #_path)


/////////////////////////////// Begin: icon specifications ////////////////////////////

///////// 工具栏顶部工具条 /////////

#define QEICN_undo QE_RSC_ICON(undo.png)   // 撤销
#define QEICN_redo QE_RSC_ICON(redo.png)   // 重做
#define QEICN_close_current QE_STD_ICON(DialogCloseButton)   // 关闭当前面板
#define QEICN_global_config QE_RSC_ICON(config.png)  // 全局配置选项下拉菜单
#define QEICN_customize QE_RSC_ICON(customize.svg)  // 自定义toolbar  (not used yet)


///////// 工具栏 开始 (1) /////////

#define QEICN_new_file QE_STD_ICON(FileIcon)            // 新建
#define QEICN_open_file QE_STD_ICON(DialogOpenButton)   // 打开
#define QEICN_save_file QE_STD_ICON(DialogSaveButton)   // 保存
#define QEICN_save_file_as QE_STD_ICON(DialogSaveAllButton)   // 另存为
#define QEICN_export_single_rail QE_RSC_ICON(ETRC-dynamic.png)  // 导出为单线路运行图

#define QEICN_navi QE_RSC_ICON(Graph-add.png)   // 运行图导航
#define QEICN_train_list QE_STD_ICON(FileDialogListView)   // 列车管理
#define QEICN_diagram_widget QE_RSC_ICON(diagram.png)      // 运行图窗口
#define QEICN_history QE_RSC_ICON(clock.png)   // 历史记录
#define QEICN_text_out_widget QE_RSC_ICON(text.png)  // 文本输出窗口
#define QEICN_issue_widget QE_STD_ICON(MessageBoxCritical)  // 问题窗口
#define QEICN_add_diagram_page QE_RSC_ICON(add.png)   // 添加运行图页面

#define QEICN_global_refresh QE_STD_ICON(BrowserReload)   // 刷新
#define QEICN_rename_station QE_RSC_ICON(brush.png)   // 更改站名

#define QEICN_help QE_STD_ICON(DialogHelpButton)   // 帮助
#define QEICN_about QE_STD_ICON(MessageBoxInformation)  // 关于
#define QEICN_about_qt QE_STD_ICON(TitleBarMenuButton)  // 关于Qt
#define QEICN_exit_app QE_STD_ICON(BrowserStop)   // 退出程序

///////// 工具栏 线路 (2) /////////

#define QEICN_import_rails QE_RSC_ICON(add.png)   // 导入线路
#define QEICN_rail_edit QE_RSC_ICON(rail.png)     // 基线编辑
#define QEICN_train_path QE_RSC_ICON(polyline.png)   // 列车径路 管理面板
#define QEICN_ruler_edit QE_RSC_ICON(ruler.phg)   // 标尺编辑
#define QEICN_skylight_edit QE_RSC_ICON(forbid.png)  // 天窗编辑
#define QEICN_new_rail QE_RSC_ICON(new-file.png)   // 新建线路

#define QEICN_read_ruler_wizard QE_RSC_ICON(ruler_pen.png)   // 标尺综合；多车次标尺读取
#define QEICN_locate_to_diagram QE_STD_ICON(FileDialogContentsView)   // 定位到运行图

#define QEICN_rail_db QE_RSC_ICON(database.png)  // 线路数据库
#define QEICN_fast_path QE_RSC_ICON(diagram.png)  // 快速径路生成
#define QEICN_route_sel QE_RSC_ICON(polyline.png)  // 经由选择

///////// 工具栏 列车 (3) /////////

#define QEICN_train_batch_op QE_RSC_ICON(copy.png)  // 列车批量操作
#define QEICN_import_trains QE_RSC_ICON(add_train.png)  // 导入车次
#define QEICN_search_train QE_STD_ICON(FileDialogContentsView)   // 搜索车次
#define QEICN_new_train QE_RSC_ICON(add.png)   // 新建车次

#define QEICN_timetable_quick QE_RSC_ICON(clock.png)   // 速览时刻窗口
#define QEICN_train_info_quick QE_RSC_ICON(info.png)   // 速览信息窗口
#define QEICN_edit_filters QE_RSC_ICON(filter.png)     // 编辑预设的列车筛选器

#define QEICN_weaken_unselect QE_RSC_ICON(weaken.png)   // 选择运行线时，背景虚化

#define QEICN_routing_edit QE_RSC_ICON(polyline.png)   // 交路编辑页面
#define QEICN_routing_batch_parse QE_RSC_ICON(text.png)  // 批量解析交路
#define QEICN_routing_batch_identify QE_RSC_ICON(identify.png)  // 批量识别交路中的车次

#define QEICN_compare_trains QE_RSC_ICON(compare.png)   // 车次对照
#define QEICN_timetable_diagon QE_RSC_ICON(identify.png)  // 时刻诊断
#define QEICN_compare_diagram QE_RSC_ICON(compare.png)  // 运行图对比
#define QEICN_section_count QE_RSC_ICON(counter.png)   // 区间对数表
#define QEICN_section_trains QE_RSC_ICON(train.png)   // 区间车次表
#define QEICN_interval_stat QE_RSC_ICON(data.png)   // 列车区间运行统计

﻿#pragma once

#include <QAbstractItemModel>
#include <QObject>
#include <QWidget>
#include <QModelIndex>

#include "data/diagram/diagram.h"
#include "componentitems.h"

/**
 * @brief The DiagramNaviModel class
 * 运行图资源管理器 用TreeView展示
 * 逻辑上，是只读的
 */
class DiagramNaviModel : public QAbstractItemModel
{
    Q_OBJECT

    Diagram& _diagram;
    std::unique_ptr<navi::DiagramItem> _root;

public:
    explicit DiagramNaviModel(Diagram& diagram, QObject* parent);

    // 通过 QAbstractItemModel 继承
    virtual QModelIndex index(int row, int column, const QModelIndex& parent = QModelIndex()) const override;
    virtual QModelIndex parent(const QModelIndex& child) const override;
    virtual int rowCount(const QModelIndex& parent = QModelIndex()) const override;
    virtual int columnCount(const QModelIndex& parent = QModelIndex()) const override;
    virtual QVariant data(const QModelIndex& index, int role = Qt::DisplayRole) const override;
    virtual QVariant headerData(int section, Qt::Orientation orientation, int role) const override;
    inline Diagram& diagram() { return _diagram; }

    /**
     * 发生了打开新运行图等操作，直接暴力重新加载Item
     */
    void resetModel();

    /**
     * 试验：导入线路，缀加到尾部。
     * 实际的添加由这里完成！！
     */
    void importRailways(const QList<std::shared_ptr<Railway>> rails);

    void removeTailRailways(int cnt);

private:
    //for convenient..
    using ACI = navi::AbstractComponentItem;
    using pACI = navi::AbstractComponentItem*;
    pACI getItem(const QModelIndex& idx)const;

signals:
    
    /**
     * 发给主窗口，打开新的编辑窗口
     */
    void newRailwayAdded(std::shared_ptr<Railway> rail);

    /**
     * 发给主窗口，关闭编辑页面
     */
    void undoneAddRailway(std::shared_ptr<Railway> rail);

    void railwayRemoved(std::shared_ptr<Railway> rail);

    void trainRemoved(std::shared_ptr<Train> train);

    void undoneTrainRemove(std::shared_ptr<Train> train);

    void newTrainAdded(std::shared_ptr<Train> train);

    void undoneAddTrain(std::shared_ptr<Train> train);

    /**
     * 一组QAbstractItemModel风格的signals，专用于通告列车插入
     * 如果直接连接默认的信号，则无法分辨是列车还是基线等的插入
     */
    void trainRowsAboutToBeInserted(int start, int end);
    void trainRowsInserted(int start,int end);
    void trainRowsAboutToBeRemoved(int start, int end);
    void trainRowsRemoved(int start, int end);

public slots:
    void resetTrainList();
    //void resetRailwayList();
    void resetPageList();

    void insertPage(std::shared_ptr<DiagramPage> page, int index);
    void removePageAt(int index);

    void onPageNameChanged(int i);

    void onRailNameChanged(std::shared_ptr<Railway> rail);

    void commitAddRailway(std::shared_ptr<Railway> rail);

    void undoAddRailway();

    void commitAddTrain(std::shared_ptr<Train> train);

    void undoAddTrain();

    void onTrainDataChanged(const QModelIndex& topleft, const QModelIndex& botright);

    void commitRemoveSingleTrain(int index);

    void undoRemoveSingleTrain(int index, std::shared_ptr<Train> train);

    /**
     * 删除由下标指定的线路数据
     * 由NaviTree调用。注意此操作不支持撤销，因此直接执行
     */
    void removeRailwayAt(int i);
};



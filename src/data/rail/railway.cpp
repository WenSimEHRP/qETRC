﻿#include "railway.h"
#include "util/qeexceptions.h"
#include <cmath>
#include <algorithm>
#include <QPair>
#include <memory>
#include <QDebug>

#include "railinterval.h"
#include "ruler.h"
#include "rulernode.h"
#include "forbid.h"
#include "data/diagram/config.h"

void RailInfoNote::fromJson(const QJsonObject& obj)
{
	author = obj.value("author").toString();
	version = obj.value("version").toString();
	note = obj.value("note").toString();
}

QJsonObject RailInfoNote::toJson() const
{
	return QJsonObject({
	   {"author",author},
	   {"version",version},
	   {"note",note}
		});
}


Railway::Railway(const QString& name) :
	_name(name), numberMapEnabled(false)
{
}

Railway::Railway(const QJsonObject& obj) :
	numberMapEnabled(false)
{
	fromJson(obj);
}

void Railway::moveStationInfo(Railway&& another)
{
	_name = std::move(another._name);
	_stations = std::move(another._stations);
	nameMap = std::move(another.nameMap);
	fieldMap = std::move(another.fieldMap);
	numberMapEnabled = another.numberMapEnabled;
	numberMap = std::move(another.numberMap);
}

void Railway::fromJson(const QJsonObject& obj)
{
	_name = obj.value("name").toString();
	_notes.fromJson(obj.value("notes").toObject());
	const QJsonArray& ar = obj.value("stations").toArray();
	for (auto t = ar.cbegin(); t != ar.cend(); t++) {
		appendStation(RailStation(t->toObject()));
	}

	//this part for Rulers
	const QJsonArray& arrulers = obj.value("rulers").toArray();
	for (auto t = arrulers.begin(); t != arrulers.end(); t++) {
		addRuler(t->toObject());
	}
    //for Forbids
    const QJsonObject& objfor=obj.value("forbid").toObject();
    addForbid(objfor);
    const QJsonObject& objfor2=obj.value("forbid2").toObject();
    addForbid(objfor2);

	_ordinate = rulerByName(obj.value("ordinate").toString());
}

QJsonObject Railway::toJson() const
{
	auto obj = QJsonObject({
		 {"name",_name},
		 {"notes",_notes.toJson()}
		});
	QJsonArray ar;
	for (const auto& t : _stations) {
		ar.append(t->toJson());
	}
	obj.insert("stations", ar);

	QJsonArray arruler;
	for (const auto& t : _rulers) {
        arruler.append(t->toJson());
	}
	obj.insert("rulers", arruler);

    //forbid
    if(_forbids.size()>=1){
        obj.insert("forbid",_forbids[0]->toJson());
    }
    if(_forbids.size()>=2){
        obj.insert("forbid2",_forbids[1]->toJson());
    }

	//排图标尺 新版
	if (_ordinate)
		obj.insert("ordinate", _ordinate->name());
	else
		obj.insert("ordinate", QJsonValue(QJsonValue::Null));

	return obj;


	/*
	todos:
	info = {
	"routes":[],
	"stations":self.stations,
	"forbid":self.forbid.outInfo(),
	"forbid2":self.forbid2.outInfo(),
	"notes":self.notes,
}

*/
}

void Railway::appendStation(const StationName& name, double mile, int level, std::optional<double> counter, PassedDirection direction)
{
	auto&& t = std::make_shared<RailStation>(
		name, mile, level, counter, direction);
	_stations.append(t);
	addMapInfo(t);
	appendInterval(t);
}

void Railway::insertStation(int index, const StationName& name, double mile,
	int level, std::optional<double> counter, PassedDirection direction)
{
	auto&& t = std::make_shared<RailStation>(name, mile, level, counter, direction);
	if (index == -1)
		_stations.append(t);
	else
		_stations.insert(index, t);
	addMapInfo(t);
	insertInterval(index, t);
}

std::shared_ptr<RailStation> Railway::stationByName(const StationName& name)
{
	return nameMap.value(name);
}

const std::shared_ptr<const RailStation> Railway::stationByName(const StationName& name) const
{
	return nameMap.value(name);
}

std::shared_ptr<RailStation>
Railway::stationByGeneralName(const StationName& name)
{
	auto p = stationByName(name);
	if (p) 
		return p;
	const QList<StationName>& t = fieldMap.value(name.station());
	for (const auto& p : t) {
		if (p.equalOrContains(name)) {
			return stationByName(p);
		}
	}
	return std::shared_ptr<RailStation>(nullptr);
}

const std::shared_ptr<const RailStation>
Railway::stationByGeneralName(const StationName& name) const
{
	auto p = stationByName(name);
	if (p)
		return p;
	const QList<StationName>& t = fieldMap.value(name.station());
	for (const auto& p : t) {
		if (p.equalOrContains(name)) {
			return stationByName(p);
		}
	}
	return std::shared_ptr<RailStation>(nullptr);
}

bool Railway::containsStation(const StationName& name) const
{
	return nameMap.contains(name);
}

bool Railway::containsGeneralStation(const StationName& name) const
{
	if (!fieldMap.contains(name.station()))
		return false;
	const auto& t = fieldMap.value(name.station());
	for (const auto& p : t) {
		if (p.isBare() || p == name)
			return true;
	}
	return false;
}

int Railway::stationIndex(const StationName& name) const
{
	if (numberMapEnabled) {
		auto t = localName(name);
		if (numberMap.contains(t))
			return numberMap.value(t);
		return -1;
	}
	else {
		return stationIndexBrute(name);
	}
}

void Railway::removeStation(const StationName& name)
{
	if (numberMapEnabled) {
		if (numberMap.contains(name)) {
			int i = numberMap.value(name);
			_stations.removeAt(i);
			numberMap.remove(name);
			removeInterval(i);
			removeMapInfo(name);
			return;
		}
	}
	//QList.remove应当是线性算法，可能还不如直接手写来得快
	for (int i = 0; i < _stations.count(); i++) {
		const auto& t = _stations[i];
		if (t->name == name) {
			removeInterval(i);
			removeMapInfo(name);
			_stations.removeAt(i);
			break;
		}
	}
}

void Railway::removeStation(int index)
{
	const auto& name = _stations.at(index)->name;
	removeInterval(index);
	_stations.removeAt(index);
	if (numberMapEnabled)
		numberMap.remove(name);
	removeMapInfo(name);
}

void Railway::adjustMileToZero()
{
	if (_stations.empty())
		return;
	double m0 = _stations[0]->mile;
	std::optional<double> c0 = _stations[0]->counter;
	for (auto& t : _stations) {
		t->mile -= m0;
	}
	if (c0.has_value()) {
		for (auto& t : _stations) {
			if (t->counter.has_value()) {
				t->counter.value() -= c0.value();
			}
		}
	}
}

Direction Railway::gapDirection(const StationName& s1, const StationName& s2) const
{
	const auto& t1 = stationByGeneralName(s1),
		t2 = stationByGeneralName(s2);
	if (!t1 || !t2)
		return Direction::Undefined;
	return t1->mile <= t2->mile ? Direction::Down : Direction::Up;
}

Direction Railway::gapDirection(const std::shared_ptr<const RailStation>& s1,
	const std::shared_ptr<const RailStation>& s2) const
{
	return s1->mile <= s2->mile ? Direction::Down : Direction::Up;
}

Direction Railway::gapDirectionByIndex(const StationName& s1, const StationName& s2) const
{
	int i1 = stationIndex(s1), i2 = stationIndex(s2);
	return i1 <= i2 ? Direction::Down : Direction::Up;
}

int Railway::stationsBetween(std::shared_ptr<const RailStation> s1, 
	std::shared_ptr<const RailStation> s2) const
{
	Direction _dir = gapDirection(s1, s2);
	//先正向找
	int cnt = 0;
	auto s = s1->dirAdjacent(_dir);
	for (; s && s != s2; s = s->dirAdjacent(_dir)) {
		cnt++;
	}
	if (s) {
		//找到了目标站
		return cnt;
	}
	//没找到，方向出了问题
	qDebug() << "Railway::stationsBetween: WARNING: invalid direction encountered " << s1->name << "->"
		<< s2->name <<", direction: "<<static_cast<int>(_dir)
		<< Qt::endl;
	cnt = 0;
	_dir = DirFunc::reverse(_dir);
	s = s1->dirAdjacent(_dir);
	for (; s && s != s2; s = s->dirAdjacent(_dir))
		cnt++;
	return cnt;
}

double Railway::mileBetween(const StationName& s1,
	const StationName& s2) const
{
	const auto& t1 = stationByGeneralName(s1),
		t2 = stationByGeneralName(s2);
	if (!t1 || !t2) {
		return -1;
	}
	if (gapDirection(t1, t2) == Direction::Up) {
		if (t1->counter.has_value() && t2->counter.has_value()) {
			return std::abs(t1->counter.value() - t2->counter.value());
		}
	}
	return std::abs(t1->mile - t2->mile);
}

bool Railway::isSplitted() const
{
	for (const auto& p : _stations) {
		if (p->direction == PassedDirection::DownVia ||
			p->direction == PassedDirection::UpVia)
			return true;
	}
	return false;
}

std::shared_ptr<Ruler> Railway::rulerByName(const QString& name)
{
	if (name.isEmpty() || name.isNull())
		return nullptr;
	for (auto p = _rulers.begin(); p != _rulers.end(); ++p) {
		if ((*p)->name() == name)
			return *p;
	}
	return std::shared_ptr<Ruler>();
}

bool Railway::rulerNameExisted(const QString& name, std::shared_ptr<const Ruler> ignore) const
{
	for (const auto& p : _rulers) {
		if (p->name() == name && p != ignore) {
			return true;
		}
	}
	return false;
}


void Railway::removeRuler(std::shared_ptr<Ruler> ruler)
{
	int index = ruler->index();
	//首先从头结点里面抹掉
	_rulers.removeAt(index);
	//改后面的序号
	for (int i = index; i < _rulers.size(); i++) {
		_rulers[i]->_index--;
	}
	//遍历interval  只需要删掉结点元素
	auto p = firstDownInterval();
	for (; p; p = nextIntervalCirc(p)) {
		p->_rulerNodes.removeAt(index);
	}
}

void Railway::clearRulers()
{
	//先去清理结点数据，免得Ruler对象被析构引起引用的危险
	for (auto p = firstDownInterval(); p; p = nextIntervalCirc(p)) {
		p->_rulerNodes.clear();
	}
	_rulers.clear();
}

void Railway::changeStationName(const StationName& oldname,
	const StationName& newname)
{
	auto p = stationByName(oldname);
	p->name = newname;

	//更新映射表
	removeMapInfo(oldname);
	addMapInfo(p);
}

double Railway::counterLength() const
{
	if (_stations.empty())
		return 0;
	const auto& last = _stations.last();
	return last->counter.has_value() ?
		last->counter.value() : last->mile;
}

void Railway::reverse()
{
	double length = railLength(), ctlen = counterLength();
    for (auto q=_stations.begin();q!=_stations.end();++q) {
        auto p=*q;
		p->mile = length - p->mile;
		if (p->counter.has_value()) {
			p->counter = ctlen - p->counter.value();
		}
		else {
			p->counter = p->mile;
		}
		std::swap(p->counter.value(), p->mile);
		switch (p->direction) {
		case PassedDirection::DownVia:
			p->direction = PassedDirection::UpVia; break;
		case PassedDirection::UpVia:
			p->direction = PassedDirection::DownVia; break;
		default:break;
		}
        std::swap(p->downNext,p->upNext);
        std::swap(p->downPrev,p->upPrev);
        //翻转每个interval内部保存的方向
        if(p->downNext){
            p->downNext->_dir=DirFunc::reverse(p->downNext->_dir);
        }
        if(p->upNext){
            p->upNext->_dir=DirFunc::reverse(p->upNext->_dir);
        }
	}
	std::reverse(_stations.begin(), _stations.end());
}

QList<QPair<StationName, StationName>> Railway::adjIntervals(bool down) const
{
	QList<QPair<StationName, StationName>> res;
	res.reserve(stationCount());   //预留空间

	if (empty()) return res;

	if (down) {
		auto p = _stations.cbegin();
		StationName last = (*p)->name;
		for (++p; p != _stations.cend(); ++p) {
			auto& q = *p;
			if (!q->isDownVia())
				continue;
			res.append(qMakePair(last, q->name));
			last = q->name;
		}
	}
	else {  //not down
		auto p = _stations.crbegin();
		StationName last = (*p)->name;
		for (++p; p != _stations.crend(); ++p) {
			auto& q = *p;
			if (!q->isUpVia())continue;
			res.append(qMakePair(last, q->name));
			last = q->name;
		}
	}
	return res;
}

void Railway::mergeCounter(const Railway& another)
{
	//注意插入会导致迭代器失效，因此不可用迭代器
	int i = 0;
	int j = another.stationCount() - 1;
	while (true) {
		if (i >= stationCount() || j < 0)
			break;
		const auto& si = _stations[i];
		const auto& sj = another._stations.at(j);
		if (si->name == sj->name) {
			si->direction = PassedDirection::BothVia;
			si->counter = another.railLength() - sj->mile;
			i++; j--;
		}
		else {
			if (!containsStation(sj->name)) {
				//上行单向站，插入处理
				double m = another.railLength() - sj->mile;
				insertStation(i, sj->name, m, sj->level,
					m, PassedDirection::UpVia);
				i++; j--;
			}
			else {
				//下行单向站
				i++;
			}
		}
	}
	//[对里程]的修正：使得零点和正里程一样
	if (empty())return;
	if (!_stations.at(0)->isUpVia()) {
		int i = 0;
		while (i < stationCount() && !_stations.at(i)->isUpVia())
			i++;
		if (i < stationCount()) {
			double m0 = _stations.at(i)->mile;
			for (int j = i; j < stationCount(); j++) {
				if (_stations.at(j)->counter.has_value()) {
					_stations.at(j)->counter.value() += m0;
				}
			}
		}
	}
}

Railway Railway::slice(int start, int end) const
{
	Railway rail;
	rail._stations.reserve(end - start);
	for (int i = start; i < end; i++) {
		rail._stations.append(_stations.at(i));
	}
	// todo ruler
	rail.setMapInfo();
	return rail;
}

void Railway::jointWith(const Railway& another, bool former, bool reverse)
{
	if (reverse) {
		this->reverse();
	}
	if (former) {
		//对方在前
		double len = another.railLength();
		for (auto& p : _stations) {
			p->mile += len;
		}
		for (auto p = another._stations.crbegin();
			p != another._stations.crend(); p++) {
			if (!containsStation((*p)->name)) {
				insertStation(0, **p);
			}
		}
	}
	else {  //not former
		double length = railLength();
		double ctlen = counterLength();
		for (const auto& t : another._stations) {
			appendStation(*t);
			auto& last = _stations.last();
			last->mile += length;
			if (last->counter.has_value())
				last->counter.value() += ctlen;
		}
	}

	//todo: 标尺天窗...
}

std::shared_ptr<RailInterval> Railway::firstDownInterval() const
{
	for (int i = 0; i < stationCount(); i++) {
		const auto& t = _stations.at(i);
		if (t->isDownVia())
			return t->downNext;
	}
	return std::shared_ptr<RailInterval>();
}

std::shared_ptr<RailInterval> Railway::firstUpInterval() const
{
	for (int i = stationCount() - 1; i >= 0; i--) {
		const auto& t = _stations.at(i);
		if (t->isUpVia()) {
			return t->upNext;
		}
	}
	return std::shared_ptr<RailInterval>();
}

void Railway::showStations() const
{
	qDebug() << "Stations list for railway: " << _name << Qt::endl;
	for (int i = 0; i < stationCount(); i++) {
		const auto& p = _stations.at(i);
		qDebug() << i << '\t' << p->name << '\t' << p->mile << '\t' <<
			p->counterStr() << '\t' <<
			static_cast<int>(p->direction) << Qt::endl;
	}
}

void Railway::showIntervals() const
{
	qDebug() << "Down intervals for railway: " << _name << Qt::endl;
	auto p = firstDownInterval();
	for (; p; p = p->nextInterval()) {
		qDebug() << p->fromStation()->name << "->" << p->toStation()->name << '\t'
			<< p->mile() << Qt::endl;
	}
	qDebug() << "Up intervals for railway: " << _name << Qt::endl;
	p = firstUpInterval();
	for (; p; p = p->nextInterval()) {
		qDebug() << p->fromStation()->name << "->" << p->toStation()->name << '\t'
			<< p->mile() << Qt::endl;
	}
}

Ruler& Railway::addEmptyRuler(const QString& name, bool different)
{
	int idx = _rulers.count();
    auto r=std::shared_ptr<Ruler>(new Ruler(*this,name,different,idx));
    _rulers.append(r);
	//下行区间
	auto p = firstDownInterval();
	for (; p; p = p->nextInterval()) {
        p->_rulerNodes.append(std::make_shared<RulerNode>(*r, *p));
	}
	p = firstUpInterval();
	for (; p; p = p->nextInterval()) {
        p->_rulerNodes.append(std::make_shared<RulerNode>(*r, *p));
	}
    return *r;
}

Ruler& Railway::addRuler(const QJsonObject& obj)
{
	const QString& name = obj.value("name").toString();
	bool dif = obj.value("different").toBool();
	Ruler& ruler = addEmptyRuler(name, dif);
	const QJsonArray& nodes = obj.value("nodes").toArray();
	for (auto p = nodes.cbegin(); p != nodes.cend(); ++p) {
		const QJsonObject& node = p->toObject();
		StationName
			from = StationName::fromSingleLiteral(node.value("fazhan").toString()),
			to = StationName::fromSingleLiteral(node.value("daozhan").toString());
		auto it = findInterval(from, to);
		if (!it) {
			qDebug() << "Railway::addRuler: WARNING: invalid interval " << from << "->" << to <<
				", to be ignored. " << Qt::endl;
		}
		else {
			it->rulerNodeAt(ruler.index())->fromJson(node);
		}
	}
	return ruler;
}

std::shared_ptr<RailInterval> Railway::findInterval(const StationName& from, const StationName& to)
{
	auto t = stationByName(from);
	if (t->hasDownAdjacent() && t->downAdjacent()->name == to)
		return t->downNext;
	else if (t->hasUpAdjacent() && t->upAdjacent()->name == to)
		return t->upNext;
	return std::shared_ptr<RailInterval>();
}

std::shared_ptr<RailInterval> Railway::findGeneralInterval(const StationName& from, const StationName& to)
{
	auto t = stationByGeneralName(from);
	//首先精确查找to
	if (t->hasDownAdjacent() && t->downAdjacent()->name == to)
		return t->downNext;
	else if (t->hasUpAdjacent() && t->upAdjacent()->name == to)
		return t->upNext;

	if (t->hasDownAdjacent() && t->downAdjacent()->name.equalOrContains(to))
		return t->downNext;
	else if (t->hasUpAdjacent() && t->upAdjacent()->name.equalOrContains(to))
		return t->upNext;
	return std::shared_ptr<RailInterval>();
}

QList<std::shared_ptr<RailInterval>> 
	Railway::multiIntervalPath(const StationName& from, const StationName& to)
{
	using ret_t = QList<std::shared_ptr<RailInterval>>;
	//如果是近邻区间，直接解决问题
	auto p = findGeneralInterval(from, to);
	if (p) {
		return ret_t{p};
	}

	auto s1 = stationByGeneralName(from), s2 = stationByGeneralName(to);
	if (!s1 || !s2)
		return ret_t();

	Direction _dir;
	if (numberMapEnabled) {
		_dir = gapDirectionByIndex(from, to);
	}
	else {
		_dir = gapDirection(s1, s2);
	}
	//不启用时：通过里程判定上下行，原则上应该八九不离十
	//但如果很不幸方向是错的，那么就是遍历完一遍之后，再来一遍
	//首先：假定这个方向是对的
	ret_t res;
	auto it = s1->dirNextInterval(_dir);
	for (; it && it->toStation() != s2; it = it->nextInterval()) {
		res.append(it);
	}
	if (it || numberMapEnabled) {
		//到最后it没有跑到空指针，说明找到了
		//如果是用index索引的，那么肯定不会错，不用考虑另一方向
		return res;
	}
	//到这里：方向是不对的，很不幸，只能重来
	qDebug() << "Railway::multiIntervalPath: WARNING: direction seems to be wrong. " <<
		from << "->" << to << Qt::endl;
	res.clear();
	_dir = DirFunc::reverse(_dir);
	it = s1->dirNextInterval(_dir);
	for (; it && it->toStation() != s2; it = it->nextInterval()) {
		res.append(it);
	}
	if (it) {
		//到最后it没有跑到空指针，说明找到了
		return res;
	}
	else {
		return ret_t();
	}
}

double Railway::calStationYValue(const Config& config)
{
	if(!ordinate())
		return calStationYValueByMile(config);
	//标尺排图
	clearYValues();
	auto ruler = ordinate();
	double y = 0;
	auto p = ruler->firstDownNode();
	p->railInterval().fromStation()->y_value = y;  //第一站
	//下行
	for (; p; p = p->nextNode()) {
		if (p->isNull()) {
			qDebug() << "Railway::calStationYValue: WARNING: "
				<< "Ruler [" << ruler->name() << "] not complate, cannot be used as"
				<< "ordinate ruler. Interval: " << p->railInterval() << Qt::endl;
			resetOrdinate();
			return calStationYValueByMile(config);
		}
		y += p->interval / config.seconds_per_pix_y;
		p->railInterval().toStation()->y_value = y;
	}
	_diagramHeight = y;
	//上行，仅补缺漏
	//这里利用了最后一站必定是双向的条件！
	for (p = ruler->firstUpNode(); p; p = p->nextNode()) {
		auto toStation = p->railInterval().toStation();
		if (!toStation->y_value.has_value()) {
			if (p->isNull()) {
				qDebug() << "Railway::calStationYValue: WARNING: "
					<< "Ruler [" << ruler->name() << "] not complate, cannot be used as"
					<< "ordinate ruler. Interval: " << p->railInterval() << Qt::endl;
				resetOrdinate();
				return calStationYValueByMile(config);
			}
			auto rboth = rightBothStation(toStation), lboth = leftBothStation(toStation);

			int upLeft = ruler->totalInterval(toStation, lboth, Direction::Up);
			int upRight = ruler->totalInterval(rboth, toStation, Direction::Up);
			double toty = rboth->y_value.value() - lboth->y_value.value();
			y = rboth->y_value.value() - toty * upRight / (upLeft + upRight);
			toStation->y_value = y;
		}
	}
	return _diagramHeight;
}

void Railway::addMapInfo(const std::shared_ptr<RailStation>& st)
{
	//nameMap  直接添加
	const auto& n = st->name;
	nameMap.insert(n, st);
	fieldMap[n.station()].append(n);
}

void Railway::removeMapInfo(const StationName& name)
{
	nameMap.remove(name);

	auto t = fieldMap.find(name.station());
	if (t == fieldMap.end())
		return;
	else if (t.value().count() == 1) {
		fieldMap.remove(name.station());
	}
	else {
		QList<StationName>& lst = t.value();
		lst.removeAll(name);
	}
}

void Railway::setMapInfo()
{
	nameMap.clear();
	fieldMap.clear();
	nameMap.reserve(stationCount());
	fieldMap.reserve(stationCount());

	for (const auto& p : _stations) {
		nameMap.insert(p->name, p);
		fieldMap[p->name.station()].append(p->name);
	}
}

void Railway::enableNumberMap()
{
	if (numberMapEnabled)
		return;
	numberMap.clear();
	for (int i = 0; i < _stations.count(); i++) {
		const auto& t = _stations[i];
		numberMap.insert(t->name, i);
	}
	numberMapEnabled = true;
}

void Railway::disableNumberMap()
{
	numberMap.clear();
	numberMapEnabled = false;
}

int Railway::stationIndexBrute(const StationName& name) const
{
	for (int i = 0; i < _stations.count(); i++) {
		if (_stations[i]->name == name)
			return i;
	}
	return -1;
}

StationName Railway::localName(const StationName& name) const
{
	const auto& t = stationByGeneralName(name);
	if (t)
		return t->name;
	else
		return name;
}

void Railway::insertStation(int i, const RailStation& station)
{
	auto&& t = std::make_shared<RailStation>(station);    //copy constructed!!
	if (i == -1)
		_stations.append(t);
	else
		_stations.insert(i, t);
	addMapInfo(t);
	insertInterval(i, t);
}

void Railway::appendStation(const RailStation& station)
{
	auto&& t = std::make_shared<RailStation>(station);    //copy constructed!!
	_stations.append(t);
	addMapInfo(t);
	appendInterval(t);
}

void Railway::appendStation(RailStation&& station)
{
	auto&& t = std::make_shared<RailStation>(std::forward<RailStation>(station));    //move constructed!!
	_stations.append(t);
	addMapInfo(t);
	appendInterval(t);
}

void Railway::appendInterval(std::shared_ptr<RailStation> st)
{
	int n = stationCount() - 1;
	if (st->isDownVia()) {
		//添加下行区间
		auto pr = leftDirStation(n, Direction::Down);
		if (pr) {
            addInterval(Direction::Down, pr, st);
		}
	}
	if (st->isUpVia()) {
		//添加上行区间
		auto pr = leftDirStation(n, Direction::Up);
		if (pr) {
            addInterval(Direction::Up, st, pr);
		}
	}
}

void Railway::insertInterval(int index, std::shared_ptr<RailStation> st)
{
	if (st->isDownVia()) {
		auto pr = leftDirStation(index, Direction::Down),
			nx = rightDirStation(index, Direction::Up);
		if (pr) {
			//前区间数据，相当于在append
            addInterval(Direction::Down, pr, st);
		}
		else {
			//后区间
            addInterval(Direction::Down, st, nx);
		}
	}
	if (st->isUpVia()) {   //上行方向
		auto pr = leftDirStation(index, Direction::Up),
			nx = rightDirStation(index, Direction::Up);
		if (pr) {
            addInterval(Direction::Up, st, pr);
		}
		if (nx) {
            addInterval(Direction::Up, nx, st);
		}
	}
}

void Railway::removeInterval(int index)
{
	auto st = _stations.at(index);
	if (st->isDownVia()) {
		auto pr = leftDirStation(index, Direction::Down),
			nx = rightDirStation(index, Direction::Down);
		if (pr && nx) {
			//区间合并
			auto it1 = st->downPrev, it2 = st->downNext;
			it1->mergeWith(*it2);
		}
		else if (pr) {
			//只有前区间，应当删除
			pr->downNext.reset();
		}
		else if (nx) {
			nx->downPrev.reset();
		}
	}
	if (st->isUpVia()) {
		auto pr = leftDirStation(index, Direction::Up),
			nx = rightDirStation(index, Direction::Up);
		if (pr && nx) {
			auto it1 = st->upPrev, it2 = st->upNext;
			it1->mergeWith(*it2);
		}
		else if (pr) {
			pr->upPrev.reset();
		}
		else if (nx) {
			nx->upNext.reset();
		}
	}
}

std::shared_ptr<RailStation> Railway::leftDirStation(int cur, Direction _dir) const
{
	for (int i = cur - 1; i >= 0; i--) {
		if (_stations.at(i)->isDirectionVia(_dir))
			return _stations.at(i);
	}
	return std::shared_ptr<RailStation>();
}

std::shared_ptr<RailStation> Railway::rightDirStation(int cur, Direction _dir) const
{
	for (int i = cur + 1; i < stationCount(); i++) {
		if (_stations.at(i)->isDirectionVia(_dir)) {
			return _stations.at(i);
		}
	}
	return std::shared_ptr<RailStation>();
}

std::shared_ptr<RailStation> Railway::leftBothStation(std::shared_ptr<RailStation> st)
{
	auto p = st->upNext;
	for (; p; p = p->nextInterval()) {
		if (p->toStation()->direction == PassedDirection::BothVia)
			return p->toStation();
	}
	qDebug() << "Railway::leftBothStation: WARNING: Unexpected null value." <<
		st->name << Qt::endl;
	return nullptr;
}

std::shared_ptr<RailStation> Railway::rightBothStation(std::shared_ptr<RailStation> st)
{
	auto p = st->upPrev;
	for (; p; p = p->prevInterval()) {
		if (p->fromStation()->direction == PassedDirection::BothVia)
			return p->fromStation();
	}
	qDebug() << "Railway::rightBothStation: WARNING: Unexpected null value. " <<
		st->name << Qt::endl;
	return nullptr;
}

std::shared_ptr<RailInterval> Railway::addInterval(Direction _dir, std::shared_ptr<RailStation> from, std::shared_ptr<RailStation> to)
{
    auto t = RailInterval::construct(_dir, from, to);
	t->_rulerNodes.reserve(_rulers.count());
	for (int i = 0; i < _rulers.count(); i++) {
        t->_rulerNodes.append(std::make_shared<RulerNode>(*_rulers[i], *t));
	}
    for(int i=0;i<_forbids.count();i++){
        t->_forbidNodes.append(std::make_shared<ForbidNode>(*_forbids[i],*t));
    }
    return t;
}

Forbid &Railway::addEmptyForbid(bool different)
{
    int n=_forbids.count();
	auto forbid = std::shared_ptr<Forbid>(new Forbid(*this, different, n));
    _forbids.append(forbid);
    auto p=firstDownInterval();
    for(;p;p=p->nextInterval()){
        p->_forbidNodes.append(std::make_shared<ForbidNode>(*forbid,*p));
    }
    p=firstUpInterval();
    for(;p;p=p->nextInterval()){
        p->_forbidNodes.append(std::make_shared<ForbidNode>(*forbid,*p));
    }
    return *forbid;
}

Forbid &Railway::addForbid(const QJsonObject &obj)
{
    bool diff=obj.value("different").toBool(true);
    Forbid& forbid=addEmptyForbid(diff);
    const QJsonArray& ar=obj.value("nodes").toArray();
    for(auto p=ar.cbegin();p!=ar.cend();++p){
        const QJsonObject& obnode=p->toObject();
        const StationName
                & from=StationName::fromSingleLiteral( obnode.value("fazhan").toString()),
                & to=StationName::fromSingleLiteral(obnode.value("daozhan").toString());
        auto pnode=forbid.getNode(from,to);
        if(!pnode){
            qDebug()<<"Railway::addForbid: WARNING: invalid interval "<<
                      from<<"->"<<to<<Qt::endl;
        }else{
            pnode->fromJson(obnode);
        }
    }
    forbid.downShow=obj.value("downShow").toBool();
    forbid.upShow=obj.value("upShow").toBool();
    return forbid;
}

std::shared_ptr<RailInterval> Railway::nextIntervalCirc(std::shared_ptr<RailInterval> railint)
{
	auto t = railint->nextInterval();
	if (!t && railint->isDown()) {
		return firstUpInterval();
	}
    return t;
}

double Railway::calStationYValueByMile(const Config& config)
{
	for (auto& p : _stations) {
		p->y_value = p->mile * config.pixels_per_km;
	}
	if (!_stations.empty())
		_diagramHeight = _stations.last()->y_value.value();
	return _diagramHeight;
}

void Railway::clearYValues()
{
	for (auto& p : _stations) {
		p->y_value = std::nullopt;
		p->clearLabelInfo();
	}
}




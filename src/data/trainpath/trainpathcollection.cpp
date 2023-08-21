#include "trainpathcollection.h"
#include <set>

#include "data/train/train.h"

void TrainPathCollection::fromJson(const QJsonArray& obj, const RailCategory& cat, TrainCollection& coll)
{
	clear();
	for (const auto& a : obj) {
		_paths.emplace_back(std::make_unique<TrainPath>(a.toObject(), cat, coll));
	}
}

QJsonArray TrainPathCollection::toJson() const
{
	QJsonArray arr{};
	for (const auto& p : _paths) {
		arr.append(p->toJson());
	}
	return arr;
}

void TrainPathCollection::clear()
{
	_paths.clear();
}

bool TrainPathCollection::isValidNewPathName(const QString& name, TrainPath* ignore) const
{
	if (name.isEmpty()) return false;
	for (const auto& p : _paths) {
		if (p->name() == name && p.get()!=ignore)
			return false;
	}
	return true;
}

QString TrainPathCollection::validNewPathName(const QString& prefix) const
{
	for (int i = 0;; i++) {
		QString name = prefix;
		if (i) {
			name += QString::number(i);
		}
		if (isValidNewPathName(name))
			return name;
	}
}

int TrainPathCollection::pathIndex(const TrainPath* p) const
{
	for (int i = 0; i < _paths.size(); i++) {
		if (_paths.at(i).get() == p) {
			return i;
		}
	}
	return -1;
}

void TrainPathCollection::checkValidAll(RailCategory& railcat)
{
	for (auto& path : _paths) {
		path->updateRailways(railcat);
		path->checkIsValid();
	}
}

void TrainPathCollection::checkValidForRailway(RailCategory& railcat, const Railway* rail)
{
	for (const auto& p : _paths) {
		if (p->containsRailway(rail)) {
			p->updateRailways(railcat);
			p->checkIsValid();
		}
	}
}

void TrainPathCollection::invalidateForRailway(const Railway* rail)
{
	for (auto& p : _paths) {
		if (p->containsRailway(rail)) {
			p->_valid = false;
		}
	}
}

std::vector<TrainPath*> TrainPathCollection::unassignedPaths(const Train& train)
{
	std::set<TrainPath*> assigned(train.paths().begin(), train.paths().end());
	std::vector<TrainPath*> res{};

	for (const auto& path : _paths) {
		auto* p = path.get();
		if (auto itr = assigned.find(p); itr == assigned.end()) {
			res.emplace_back(p);
		}
	}
	return res;
}

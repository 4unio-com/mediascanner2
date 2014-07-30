/*
 * Copyright (C) 2014 Canonical, Ltd.
 *
 * Authors:
 *    Jussi Pakkanen <jussi.pakkanen@canonical.com>
 *
 * This library is free software; you can redistribute it and/or modify it under
 * the terms of version 3 of the GNU General Public License as published
 * by the Free Software Foundation.
 *
 * This library is distributed in the hope that it will be useful, but WITHOUT
 * ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS
 * FOR A PARTICULAR PURPOSE. See the GNU General Public License for more
 * details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

#include "StreamingModel.hh"

#include <cassert>

#include <QEvent>
#include <QCoreApplication>
#include <QScopedPointer>
#include <QtConcurrent>

using namespace mediascanner::qml;

namespace {

const int BATCH_SIZE = 200;

class AdditionEvent : public QEvent {
private:
    std::unique_ptr<StreamingModel::RowData> rows;
    int generation;

public:
    AdditionEvent(std::unique_ptr<StreamingModel::RowData> &&rows, int generation) :
        QEvent(AdditionEvent::additionEventType()), rows(std::move(rows)), generation(generation) {
    }

    std::unique_ptr<StreamingModel::RowData>& getRows() { return rows; }
    int getGeneration() const { return generation; }

    static QEvent::Type additionEventType()
    {
        static QEvent::Type type = static_cast<QEvent::Type>(QEvent::registerEventType());
        return type;
    }
};

void runQuery(int generation, StreamingModel *model, std::shared_ptr<mediascanner::MediaStoreBase> store) {
    if (!store) {
        return;
    }
    int offset = 0;
    int cursize;
    do {
        if(model->shouldWorkerStop()) {
            return;
        }
        QScopedPointer<AdditionEvent> e(
            new AdditionEvent(model->retrieveRows(store, BATCH_SIZE, offset), generation));
        cursize = e->getRows()->size();
        if (model->shouldWorkerStop()) {
            return;
        }
        QCoreApplication::instance()->postEvent(model, e.take());
        offset += cursize;
    } while(cursize >= BATCH_SIZE);
}

}

StreamingModel::StreamingModel(QObject *parent) : QAbstractListModel(parent), generation(0) {
}

StreamingModel::~StreamingModel() {
    setWorkerStop(true);
    try {
        query_future.waitForFinished();
    } catch(...) {
        qWarning() << "Unknown error when shutting down worker thread.\n";
    }
}

void StreamingModel::updateModel() {
    if (store.isNull()) {
        query_future = QFuture<void>();
        Q_EMIT filled();
        return;
    }
    setWorkerStop(false);
    query_future = QtConcurrent::run(runQuery, ++generation, this, store->store);
}

QVariant StreamingModel::get(int row, int role) const {
    return data(index(row, 0), role);
}

bool StreamingModel::event(QEvent *e) {
    if (e->type() != AdditionEvent::additionEventType()) {
        return QObject::event(e);
    }
    AdditionEvent *ae = dynamic_cast<AdditionEvent*>(e);
    assert(ae);

    // Old results may be in Qt's event queue and get delivered
    // after we have moved to a new query. Ignore them.
    if(ae->getGeneration() != generation) {
        return true;
    }

    auto &newrows = ae->getRows();
    bool lastBatch = newrows->size() < BATCH_SIZE;
    beginInsertRows(QModelIndex(), rowCount(), rowCount()+newrows->size());
    appendRows(std::move(newrows));
    endInsertRows();
    Q_EMIT rowCountChanged();
    if (lastBatch) {
        Q_EMIT filled();
    }
    return true;
}

MediaStoreWrapper *StreamingModel::getStore() {
    return store.data();
}

void StreamingModel::setStore(MediaStoreWrapper *store) {
    if (this->store != store) {
        this->store = store;
        invalidate();
    }
}

void StreamingModel::invalidate() {
    setWorkerStop(true);
    query_future.waitForFinished();
    beginResetModel();
    clearBacking();
    endResetModel();
    Q_EMIT rowCountChanged();
    updateModel();
}
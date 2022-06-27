// Copyright 2015 Citra Emulator Project
// Licensed under GPLv2 or any later version
// Refer to the license.txt file included.

#pragma once

#include <QMenu>
#include <QString>
#include <QWidget>
#include "citra_qt/compatibility_list.h"
#include "common/common_types.h"
#include "uisettings.h"

class GameListWorker;
class GameListDir;
class GameListSearchField;
class GMainWindow;
class QFileSystemWatcher;
class QHBoxLayout;
class QLabel;
class QLineEdit;
template <typename>
class QList;
class QModelIndex;
class QStandardItem;
class QStandardItemModel;
class QTreeView;
class QToolButton;
class QVBoxLayout;

enum class GameListOpenTarget {
    SAVE_DATA = 0,
    EXT_DATA = 1,
    APPLICATION = 2,
    UPDATE_DATA = 3,
    TEXTURE_DUMP = 4,
    TEXTURE_LOAD = 5,
    MODS = 6,
};

class GameList : public QWidget {
    Q_OBJECT

public:
    enum {
        COLUMN_NAME,
        COLUMN_COMPATIBILITY,
        COLUMN_REGION,
        COLUMN_FILE_TYPE,
        COLUMN_SIZE,
        COLUMN_COUNT, // Number of columns
    };

    explicit GameList(GMainWindow* parent = nullptr);
    ~GameList() override;

    QString getLastFilterResultItem();
    void clearFilter();
    void setFilterFocus();
    void setFilterVisible(bool visibility);
    void setDirectoryWatcherEnabled(bool enabled);
    bool isEmpty();

    void LoadCompatibilityList();
    void PopulateAsync(QList<UISettings::GameDir>& game_dirs);

    void SaveInterfaceLayout();
    void LoadInterfaceLayout();

    QStandardItemModel* GetModel() const;

    QString FindGameByProgramID(u64 program_id);

    void RefreshGameDirectory();

    static const QStringList supported_file_extensions;

signals:
    void GameChosen(QString game_path);
    void ShouldCancelWorker();
    void OpenFolderRequested(u64 program_id, GameListOpenTarget target);
    void NavigateToGamedbEntryRequested(u64 program_id,
                                        const CompatibilityList& compatibility_list);
    void DumpRomFSRequested(QString game_path, u64 program_id);
    void OpenDirectory(QString directory);
    void AddDirectory();
    void ShowList(bool show);
    void PopulatingCompleted();

private slots:
    void onItemExpanded(const QModelIndex& item);
    void onTextChanged(const QString& newText);
    void onFilterCloseClicked();
    void onUpdateThemedIcons();

private:
    void AddDirEntry(GameListDir* entry_items);
    void AddEntry(const QList<QStandardItem*>& entry_items, GameListDir* parent);
    void ValidateEntry(const QModelIndex& item);
    void DonePopulating(QStringList watch_list);

    void PopupContextMenu(const QPoint& menu_location);
    void AddGamePopup(QMenu& context_menu, const QString& path, u64 program_id, u64 extdata_id);
    void AddCustomDirPopup(QMenu& context_menu, QModelIndex selected);
    void AddPermDirPopup(QMenu& context_menu, QModelIndex selected);

    QString FindGameByProgramID(QStandardItem* current_item, u64 program_id);

    GameListSearchField* search_field;
    GMainWindow* main_window = nullptr;
    QVBoxLayout* layout = nullptr;
    QTreeView* tree_view = nullptr;
    QStandardItemModel* item_model = nullptr;
    GameListWorker* current_worker = nullptr;
    QFileSystemWatcher* watcher = nullptr;
    CompatibilityList compatibility_list;

    friend class GameListSearchField;
};

Q_DECLARE_METATYPE(GameListOpenTarget);

class GameListPlaceholder : public QWidget {
    Q_OBJECT
public:
    explicit GameListPlaceholder(GMainWindow* parent = nullptr);
    ~GameListPlaceholder();

signals:
    void AddDirectory();

private slots:
    void onUpdateThemedIcons();

protected:
    void mouseDoubleClickEvent(QMouseEvent* event) override;

private:
    GMainWindow* main_window = nullptr;
    QVBoxLayout* layout = nullptr;
    QLabel* image = nullptr;
    QLabel* text = nullptr;
};

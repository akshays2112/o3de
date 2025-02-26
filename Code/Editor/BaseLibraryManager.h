/*
 * Copyright (c) Contributors to the Open 3D Engine Project.
 * For complete copyright and license terms please see the LICENSE at the root of this distribution.
 *
 * SPDX-License-Identifier: Apache-2.0 OR MIT
 *
 */



#ifndef CRYINCLUDE_EDITOR_BASELIBRARYMANAGER_H
#define CRYINCLUDE_EDITOR_BASELIBRARYMANAGER_H
#pragma once

#include "Include/IBaseLibraryManager.h"
#include "Include/IDataBaseItem.h"
#include "Include/IDataBaseLibrary.h"
#include "Include/IDataBaseManager.h"
#include "Util/TRefCountBase.h"
#include "Util/GuidUtil.h"
#include "BaseLibrary.h"
#include "Util/smartptr.h"
#include <EditorDefs.h>
#include <QtUtil.h>

AZ_PUSH_DISABLE_DLL_EXPORT_BASECLASS_WARNING
/** Manages all Libraries and Items.
*/
class SANDBOX_API  CBaseLibraryManager
    : public IBaseLibraryManager
{
AZ_POP_DISABLE_DLL_EXPORT_BASECLASS_WARNING
public:
    CBaseLibraryManager();
    ~CBaseLibraryManager();

    //! Clear all libraries.
    virtual void ClearAll() override;

    //////////////////////////////////////////////////////////////////////////
    // IDocListener implementation.
    //////////////////////////////////////////////////////////////////////////
    virtual void OnEditorNotifyEvent(EEditorNotifyEvent event) override;

    //////////////////////////////////////////////////////////////////////////
    // Library items.
    //////////////////////////////////////////////////////////////////////////
    //! Make a new item in specified library.
    virtual IDataBaseItem* CreateItem(IDataBaseLibrary* pLibrary) override;
    //! Delete item from library and manager.
    virtual void DeleteItem(IDataBaseItem* pItem) override;

    //! Find Item by its GUID.
    virtual IDataBaseItem* FindItem(REFGUID guid) const;
    virtual IDataBaseItem* FindItemByName(const QString& fullItemName);
    virtual IDataBaseItem* LoadItemByName(const QString& fullItemName);
    virtual IDataBaseItem* FindItemByName(const char* fullItemName);
    virtual IDataBaseItem* LoadItemByName(const char* fullItemName);

    virtual IDataBaseItemEnumerator* GetItemEnumerator() override;

    //////////////////////////////////////////////////////////////////////////
    // Set item currently selected.
    virtual void SetSelectedItem(IDataBaseItem* pItem) override;
    // Get currently selected item.
    virtual IDataBaseItem* GetSelectedItem() const override;
    virtual IDataBaseItem* GetSelectedParentItem() const override;

    //////////////////////////////////////////////////////////////////////////
    // Libraries.
    //////////////////////////////////////////////////////////////////////////
    //! Add Item library.
    virtual IDataBaseLibrary* AddLibrary(const QString& library, bool bIsLevelLibrary = false, bool bIsLoading = true) override;
    virtual void DeleteLibrary(const QString& library, bool forceDeleteLevel = false) override;
    //! Get number of libraries.
    virtual int GetLibraryCount() const override { return static_cast<int>(m_libs.size()); };
    //! Get number of modified libraries.
    virtual int GetModifiedLibraryCount() const override;

    //! Get Item library by index.
    virtual IDataBaseLibrary* GetLibrary(int index) const override;

    //! Get Level Item library.
    virtual IDataBaseLibrary* GetLevelLibrary() const override;

    //! Find Items Library by name.
    virtual IDataBaseLibrary* FindLibrary(const QString& library) override;

    //! Find Items Library's index by name.
    int FindLibraryIndex(const QString& library) override;
    
    //! Load Items library.
    virtual IDataBaseLibrary* LoadLibrary(const QString& filename, bool bReload = false) override;

    //! Save all modified libraries.
    virtual void SaveAllLibs() override;

    //! Serialize property manager.
    virtual void Serialize(XmlNodeRef& node, bool bLoading) override;

    //! Export items to game.
    virtual void Export([[maybe_unused]] XmlNodeRef& node) override {};

    //! Returns unique name base on input name.
    virtual QString MakeUniqueItemName(const QString& name, const QString& libName = "") override;
    virtual QString MakeFullItemName(IDataBaseLibrary* pLibrary, const QString& group, const QString& itemName) override;

    //! Root node where this library will be saved.
    virtual QString GetRootNodeName() override = 0;
    //! Path to libraries in this manager.
    virtual QString GetLibsPath() override = 0;

    //////////////////////////////////////////////////////////////////////////
    //! Validate library items for errors.
    virtual void Validate() override;

    //////////////////////////////////////////////////////////////////////////
    virtual void GatherUsedResources(CUsedResources& resources) override;

    virtual void AddListener(IDataBaseManagerListener* pListener) override;
    virtual void RemoveListener(IDataBaseManagerListener* pListener) override;

    //////////////////////////////////////////////////////////////////////////
    virtual void RegisterItem(CBaseLibraryItem* pItem, REFGUID newGuid) override;
    virtual void RegisterItem(CBaseLibraryItem* pItem) override;
    virtual void UnregisterItem(CBaseLibraryItem* pItem) override;

    // Only Used internally.
    virtual void OnRenameItem(CBaseLibraryItem* pItem, const QString& oldName) override;

    // Called by items to indicated that they have been modified.
    // Sends item changed event to listeners.
    virtual void OnItemChanged(IDataBaseItem* pItem) override;
    virtual void OnUpdateProperties(IDataBaseItem* pItem, bool bRefresh) override;

    QString MakeFilename(const QString& library);
    virtual bool IsUniqueFilename(const QString& library) override;

    //CONFETTI BEGIN
    // Used to change the library item order
    virtual void ChangeLibraryOrder(IDataBaseLibrary* lib, unsigned int newLocation) override;

    virtual bool SetLibraryName(CBaseLibrary* lib, const QString& name) override;

protected:
    void SplitFullItemName(const QString& fullItemName, QString& libraryName, QString& itemName);
    void NotifyItemEvent(IDataBaseItem* pItem, EDataBaseItemEvent event);
    void SetRegisteredFlag(CBaseLibraryItem* pItem, bool bFlag);

    //////////////////////////////////////////////////////////////////////////
    // Must be overriden.
    //! Makes a new Item.
    virtual CBaseLibraryItem* MakeNewItem() = 0;
    virtual CBaseLibrary* MakeNewLibrary() = 0;
    //////////////////////////////////////////////////////////////////////////

    virtual void ReportDuplicateItem(CBaseLibraryItem* pItem, CBaseLibraryItem* pOldItem);

protected:
    bool m_bUniqGuidMap;
    bool m_bUniqNameMap;

    AZ_PUSH_DISABLE_DLL_EXPORT_MEMBER_WARNING
    //! Array of all loaded entity items libraries.
    std::vector<_smart_ptr<CBaseLibrary> > m_libs;

    // There is always one current level library.
    TSmartPtr<CBaseLibrary> m_pLevelLibrary;

    // GUID to item map.
    typedef std::map<GUID, _smart_ptr<CBaseLibraryItem>, guid_less_predicate> ItemsGUIDMap;
    ItemsGUIDMap m_itemsGuidMap;

    // Case insensitive name to items map.
    typedef std::map<QString, _smart_ptr<CBaseLibraryItem>, stl::less_stricmp<QString>> ItemsNameMap;
    ItemsNameMap m_itemsNameMap;
    AZStd::mutex m_itemsNameMapMutex;

    std::vector<IDataBaseManagerListener*> m_listeners;

    // Currently selected item.
    _smart_ptr<CBaseLibraryItem> m_pSelectedItem;
    _smart_ptr<CBaseLibraryItem> m_pSelectedParent;
    AZ_POP_DISABLE_DLL_EXPORT_MEMBER_WARNING
};

//////////////////////////////////////////////////////////////////////////
template <class TMap>
class CDataBaseItemEnumerator
    : public IDataBaseItemEnumerator
{
    TMap* m_pMap;
    typename TMap::iterator m_iterator;

public:
    CDataBaseItemEnumerator(TMap* pMap)
    {
        assert(pMap);
        m_pMap = pMap;
        m_iterator = m_pMap->begin();
    }
    virtual void Release() { delete this; };
    virtual IDataBaseItem* GetFirst()
    {
        m_iterator = m_pMap->begin();
        if (m_iterator == m_pMap->end())
        {
            return 0;
        }
        return m_iterator->second;
    }
    virtual IDataBaseItem* GetNext()
    {
        if (m_iterator != m_pMap->end())
        {
            m_iterator++;
        }
        if (m_iterator == m_pMap->end())
        {
            return 0;
        }
        return m_iterator->second;
    }
};

#endif // CRYINCLUDE_EDITOR_BASELIBRARYMANAGER_H

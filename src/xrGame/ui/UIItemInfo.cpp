#include "pch_script.h"
#include "uiiteminfo.h"
#include "xrUICore/Static/UIStatic.h"
#include "UIXmlInit.h"
#include "xrUICore/ProgressBar/UIProgressBar.h"
#include "xrUICore/ScrollView/UIScrollView.h"
#include "xrUICore/Windows/UIFrameWindow.h"
#include "ai_space.h"
#include "alife_simulator.h"
#include "string_table.h"
#include "Inventory_Item.h"
#include "UIInventoryUtilities.h"
#include "PhysicsShellHolder.h"
#include "UIWpnParams.h"
#include "ui_af_params.h"
#include "UIInvUpgradeProperty.h"
#include "UIOutfitInfo.h"
#include "UIBoosterInfo.h"
#include "Weapon.h"
#include "CustomOutfit.h"
#include "ActorHelmet.h"
#include "Artefact.h"
#include "ActorBackpack.h"
#include "eatable_item.h"
#include "UICellItem.h"
#include "xrGame/game_type.h"

extern const LPCSTR g_inventory_upgrade_xml;

#define INV_GRID_WIDTH2 40.0f
#define INV_GRID_HEIGHT2 40.0f

CUIItemInfo::CUIItemInfo()
{
    UIItemImageSize.set(0.0f, 0.0f);

    UICost = NULL;
    UITradeTip = NULL;
    UIWeight = NULL;
    UIItemImage = NULL;
    UIDesc = NULL;
    UIWpnParams = NULL;
    UIProperties = NULL;
    UIOutfitInfo = NULL;
    UIBoosterInfo = NULL;
    UIArtefactParams = NULL;
    UIName = NULL;
    UIBackground = NULL;
    m_pInvItem = NULL;
    m_b_FitToHeight = false;
    m_complex_desc = false;
}

CUIItemInfo::~CUIItemInfo()
{
    xr_delete(UIWpnParams);
    xr_delete(UIArtefactParams);
    xr_delete(UIProperties);
    xr_delete(UIOutfitInfo);
    xr_delete(UIBoosterInfo);
}

void CUIItemInfo::InitItemInfo(LPCSTR xml_name)
{
    CUIXml uiXml;
    uiXml.Load(CONFIG_PATH, UI_PATH, UI_PATH_DEFAULT, xml_name);
    CUIXmlInit xml_init;

    if (uiXml.NavigateToNode("main_frame", 0))
    {
        Frect wnd_rect;
        wnd_rect.x1 = uiXml.ReadAttribFlt("main_frame", 0, "x", 0);
        wnd_rect.y1 = uiXml.ReadAttribFlt("main_frame", 0, "y", 0);

        wnd_rect.x2 = uiXml.ReadAttribFlt("main_frame", 0, "width", 0);
        wnd_rect.y2 = uiXml.ReadAttribFlt("main_frame", 0, "height", 0);
        wnd_rect.x2 += wnd_rect.x1;
        wnd_rect.y2 += wnd_rect.y1;
        inherited::SetWndRect(wnd_rect);

        delay = uiXml.ReadAttribInt("main_frame", 0, "delay", 500);
    }

    if (uiXml.NavigateToNode("background_frame", 0))
    {
        UIBackground = new CUIFrameWindow();
        UIBackground->SetAutoDelete(true);
        AttachChild(UIBackground);
        xml_init.InitFrameWindow(uiXml, "background_frame", 0, UIBackground);
    }

    m_complex_desc = false;
    if (uiXml.NavigateToNode("static_name", 0))
    {
        UIName = new CUITextWnd();
        AttachChild(UIName);
        UIName->SetAutoDelete(true);
        xml_init.InitTextWnd(uiXml, "static_name", 0, UIName);
        m_complex_desc = (uiXml.ReadAttribInt("static_name", 0, "complex_desc", 0) == 1);
    }

    if (uiXml.NavigateToNode("static_weight", 0))
    {
        UIWeight = new CUITextWnd();
        AttachChild(UIWeight);
        UIWeight->SetAutoDelete(true);
        xml_init.InitTextWnd(uiXml, "static_weight", 0, UIWeight);
    }

    if (uiXml.NavigateToNode("static_cost", 0))
    {
        UICost = new CUITextWnd();
        AttachChild(UICost);
        UICost->SetAutoDelete(true);
        xml_init.InitTextWnd(uiXml, "static_cost", 0, UICost);
    }

    if (uiXml.NavigateToNode("static_no_trade", 0))
    {
        UITradeTip = new CUITextWnd();
        AttachChild(UITradeTip);
        UITradeTip->SetAutoDelete(true);
        xml_init.InitTextWnd(uiXml, "static_no_trade", 0, UITradeTip);
    }

    if (uiXml.NavigateToNode("descr_list", 0))
    {
        UIWpnParams = new CUIWpnParams();
        UIWpnParams->InitFromXml(uiXml);

        UIArtefactParams = new CUIArtefactParams();
        UIArtefactParams->InitFromXml(uiXml);

        UIBoosterInfo = new CUIBoosterInfo();
        UIBoosterInfo->InitFromXml(uiXml);

        if (ai().get_alife()) // (-designer)
        {
            UIProperties = new UIInvUpgPropertiesWnd();
            UIProperties->init_from_xml("actor_menu_item.xml");
        }

        UIDesc = new CUIScrollView();
        AttachChild(UIDesc);
        UIDesc->SetAutoDelete(true);
        m_desc_info.bShowDescrText = !!uiXml.ReadAttribInt("descr_list", 0, "only_text_info", 1);
        m_b_FitToHeight = !!uiXml.ReadAttribInt("descr_list", 0, "fit_to_height", 0);
        xml_init.InitScrollView(uiXml, "descr_list", 0, UIDesc);
        xml_init.InitFont(uiXml, "descr_list:font", 0, m_desc_info.uDescClr, m_desc_info.pDescFont);
    }

    if (uiXml.NavigateToNode("image_static", 0))
    {
        UIItemImage = new CUIStatic();
        AttachChild(UIItemImage);
        UIItemImage->SetAutoDelete(true);
        xml_init.InitStatic(uiXml, "image_static", 0, UIItemImage);
        UIItemImage->TextureOn();

        UIItemImage->TextureOff();
        UIItemImageSize.set(UIItemImage->GetWidth(), UIItemImage->GetHeight());
    }
    if (uiXml.NavigateToNode("outfit_info", 0))
    {
        UIOutfitInfo = new CUIOutfitInfo();
        UIOutfitInfo->InitFromXml(uiXml);
    }

    xml_init.InitAutoStaticGroup(uiXml, "auto", 0, this);
}

void CUIItemInfo::InitItemInfo(Fvector2 pos, Fvector2 size, LPCSTR xml_name)
{
    inherited::SetWndPos(pos);
    inherited::SetWndSize(size);
    InitItemInfo(xml_name);
}

void CUIItemInfo::InitItem(CUICellItem* pCellItem, CInventoryItem* pCompareItem, u32 item_price, LPCSTR trade_tip)
{
    if (!pCellItem)
    {
        m_pInvItem = NULL;
        Enable(false);
        return;
    }

    PIItem pInvItem = (PIItem)pCellItem->m_pData;
    m_pInvItem = pInvItem;
    Enable(NULL != m_pInvItem);
    if (!m_pInvItem)
        return;

    Fvector2 pos;
    pos.set(0.0f, 0.0f);
    string256 str;
    if (UIName)
    {
        UIName->SetText(pInvItem->NameItem());
        UIName->AdjustHeightToText();
        pos.y = UIName->GetWndPos().y + UIName->GetHeight() + 4.0f;
    }
    if (UIWeight)
    {
        LPCSTR kg_str = StringTable().translate("st_kg").c_str();
        float weight = pInvItem->Weight();

        if (!weight)
        {
            if (CWeaponAmmo* ammo = dynamic_cast<CWeaponAmmo*>(pInvItem))
            {
                weight = pInvItem->CInventoryItem::Weight();
                for (u32 j = 0; j < pCellItem->ChildsCount(); ++j)
                {
                    PIItem jitem = (PIItem)pCellItem->Child(j)->m_pData;
                    weight += jitem->CInventoryItem::Weight();
                }
            }
        }

        xr_sprintf(str, "%3.2f %s", weight, kg_str);
        UIWeight->SetText(str);

        pos.x = UIWeight->GetWndPos().x;
        if (m_complex_desc)
        {
            UIWeight->SetWndPos(pos);
        }
    }
	
	if ( UICost && IsGameTypeSingle() )
	{
		sprintf				(str, "%d RU", pInvItem->Cost());	
		UICost->SetText		(str);
		pos.x = UICost->GetWndPos().x;
		if ( m_complex_desc )
		{
			UICost->SetWndPos	(pos);
		}
	}

    if (UITradeTip)
    {
        pos.y = UITradeTip->GetWndPos().y;
        if (UIWeight && m_complex_desc)
        {
            pos.y = UIWeight->GetWndPos().y + UIWeight->GetHeight() + 4.0f;
        }

        if (trade_tip == NULL)
            UITradeTip->Show(false);
        else
        {
            UITradeTip->SetText(StringTable().translate(trade_tip).c_str());
            UITradeTip->AdjustHeightToText();
            UITradeTip->SetWndPos(pos);
            UITradeTip->Show(true);
        }
    }

    if (UIDesc)
    {
        pos = UIDesc->GetWndPos();
        if (UIWeight)
            pos.y = UIWeight->GetWndPos().y + UIWeight->GetHeight() + 4.0f;

        if (UITradeTip && trade_tip != NULL)
            pos.y = UITradeTip->GetWndPos().y + UITradeTip->GetHeight() + 4.0f;

        UIDesc->SetWndPos(pos);
        UIDesc->Clear();
        VERIFY(0 == UIDesc->GetSize());
        if (m_desc_info.bShowDescrText)
        {
            CUITextWnd* pItem = new CUITextWnd();
            pItem->SetTextColor(m_desc_info.uDescClr);
            pItem->SetFont(m_desc_info.pDescFont);
            pItem->SetWidth(UIDesc->GetDesiredChildWidth());
            pItem->SetTextComplexMode(true);
            pItem->SetText(*pInvItem->ItemDescription());
            pItem->AdjustHeightToText();
            UIDesc->AddWindow(pItem, true);
        }
        TryAddConditionInfo(*pInvItem, pCompareItem);
        TryAddWpnInfo(*pInvItem, pCompareItem);
        TryAddArtefactInfo(*pInvItem);
        TryAddOutfitInfo(*pInvItem, pCompareItem);
        TryAddUpgradeInfo(*pInvItem);
        TryAddBoosterInfo(*pInvItem);

        if (m_b_FitToHeight)
        {
            UIDesc->SetWndSize(Fvector2().set(UIDesc->GetWndSize().x, UIDesc->GetPadSize().y));
            Fvector2 new_size;
            new_size.x = GetWndSize().x;
            new_size.y = UIDesc->GetWndPos().y + UIDesc->GetWndSize().y + 20.0f;
            new_size.x = _max(105.0f, new_size.x);
            new_size.y = _max(105.0f, new_size.y);

            SetWndSize(new_size);
            if (UIBackground)
                UIBackground->SetWndSize(new_size);
        }

        UIDesc->ScrollToBegin();
    }
    if (UIItemImage)
    {
        // Загружаем картинку
        UIItemImage->SetShader(InventoryUtilities::GetEquipmentIconsShader());

        Irect item_grid_rect = pInvItem->GetInvGridRect();
        Frect texture_rect;
        texture_rect.lt.set(item_grid_rect.x1 * INV_GRID_WIDTH, item_grid_rect.y1 * INV_GRID_HEIGHT);
        texture_rect.rb.set(item_grid_rect.x2 * INV_GRID_WIDTH, item_grid_rect.y2 * INV_GRID_HEIGHT);
        texture_rect.rb.add(texture_rect.lt);
        UIItemImage->GetUIStaticItem().SetTextureRect(texture_rect);
        UIItemImage->TextureOn();
        UIItemImage->SetStretchTexture(true);
        Fvector2 v_r = {item_grid_rect.x2 * INV_GRID_WIDTH2, item_grid_rect.y2 * INV_GRID_HEIGHT2};

        v_r.x *= UI().get_current_kx();

        UIItemImage->GetUIStaticItem().SetSize(v_r);
        UIItemImage->SetWidth(v_r.x);
        UIItemImage->SetHeight(v_r.y);
    }
}

void CUIItemInfo::TryAddConditionInfo(CInventoryItem& pInvItem, CInventoryItem* pCompareItem)
{
    CWeapon* weapon = smart_cast<CWeapon*>(&pInvItem);
    CCustomOutfit* outfit = smart_cast<CCustomOutfit*>(&pInvItem);
    if (weapon || outfit) {}
}

void CUIItemInfo::TryAddWpnInfo(CInventoryItem& pInvItem, CInventoryItem* pCompareItem)
{
    if (UIWpnParams->Check(pInvItem))
    {
        UIWpnParams->SetInfo(pCompareItem, pInvItem);
        UIDesc->AddWindow(UIWpnParams, false);
    }
}

void CUIItemInfo::TryAddArtefactInfo(CInventoryItem& pInvItem)
{
    if (!UIArtefactParams) return;
 	CCustomOutfit* outfit = smart_cast<CCustomOutfit*>(&pInvItem);
	CHelmet* helmet = smart_cast<CHelmet*>(&pInvItem);
	CBackpack* backpack = smart_cast<CBackpack*>(&pInvItem);
	CArtefact* arte = smart_cast<CArtefact*>(&pInvItem);
	if (outfit)
    {
        UIArtefactParams->SetInfo(outfit);
        UIDesc->AddWindow(UIArtefactParams, false);
    }
    else if (arte)
    {
        UIArtefactParams->SetInfo(arte);
        UIDesc->AddWindow(UIArtefactParams, false);
    }
    else if (helmet)
    {
        UIArtefactParams->SetInfo(helmet);
        UIDesc->AddWindow(UIArtefactParams, false);
    }
    else if (backpack)
    {
        UIArtefactParams->SetInfo(backpack);
        UIDesc->AddWindow(UIArtefactParams, false);
    }
}

void CUIItemInfo::TryAddOutfitInfo(CInventoryItem& pInvItem, CInventoryItem* pCompareItem)
{
    if (!UIOutfitInfo) return;

    CCustomOutfit* outfit = smart_cast<CCustomOutfit*>(&pInvItem);
    CHelmet* helmet = smart_cast<CHelmet*>(&pInvItem);
    if (outfit)
    {
        CCustomOutfit* comp_outfit = smart_cast<CCustomOutfit*>(pCompareItem);
        UIOutfitInfo->UpdateInfo(outfit, comp_outfit);
        UIDesc->AddWindow(UIOutfitInfo, false);
    }
    else if (helmet)
    {
        CHelmet* comp_helmet = smart_cast<CHelmet*>(pCompareItem);
        UIOutfitInfo->UpdateInfo(helmet, comp_helmet);
        UIDesc->AddWindow(UIOutfitInfo, false);
    }
}

void CUIItemInfo::TryAddUpgradeInfo(CInventoryItem& pInvItem)
{
    if (pInvItem.upgardes().size() && UIProperties)
    {
        UIProperties->set_item_info(pInvItem);
        UIDesc->AddWindow(UIProperties, false);
    }
}

void CUIItemInfo::TryAddBoosterInfo(CInventoryItem& pInvItem)
{
    CEatableItem* food = smart_cast<CEatableItem*>(&pInvItem);
    if (food && UIBoosterInfo)
    {
        UIBoosterInfo->SetInfo(pInvItem.object().cNameSect());
        UIDesc->AddWindow(UIBoosterInfo, false);
    }
}

void CUIItemInfo::Draw()
{
    if (m_pInvItem)
        inherited::Draw();
}
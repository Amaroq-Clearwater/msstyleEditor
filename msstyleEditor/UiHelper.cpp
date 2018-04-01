#include "UiHelper.h"
#include "libmsstyle\VisualStyle.h"
#include "libmsstyle\VisualStyleEnums.h"
#include "libmsstyle\VisualStyleDefinitions.h"

#include <string>
#include <codecvt>	// codecvt_utf8_utf16
#include <locale>	// wstring_convert

using namespace libmsstyle;

wxMenu* BuildPropertyMenu(int idbase)
{
	int ID_BASE_PROP = idbase;
	wxMenu* createPropMenu = new wxMenu();

	wxMenu* enums = new wxMenu();
	for (int i = IDENTIFIER::BORDERTYPE; i <= IDENTIFIER::TRUESIZESCALINGTYPE; ++i)
	{
		enums->Append(ID_BASE_PROP + i, libmsstyle::lookup::FindPropertyName(i));
	}

	wxMenu* margins = new wxMenu();
	margins->Append(ID_BASE_PROP + IDENTIFIER::SIZINGMARGINS, lookup::FindPropertyName(IDENTIFIER::SIZINGMARGINS));
	margins->Append(ID_BASE_PROP + IDENTIFIER::CONTENTMARGINS, lookup::FindPropertyName(IDENTIFIER::CONTENTMARGINS));
	margins->Append(ID_BASE_PROP + IDENTIFIER::CAPTIONMARGINS, lookup::FindPropertyName(IDENTIFIER::CAPTIONMARGINS));

	wxMenu* colors = new wxMenu();
	for (int i = IDENTIFIER::FIRSTCOLOR; i <= IDENTIFIER::LASTCOLOR; ++i)
	{
		colors->Append(ID_BASE_PROP + i, libmsstyle::lookup::FindPropertyName(i));
	}

	createPropMenu->AppendSubMenu(enums, "Enums");
	createPropMenu->AppendSubMenu(margins, "Margins");
	createPropMenu->AppendSubMenu(colors, "Colors");
	return createPropMenu;
}

wxPGProperty* GetWXPropertyFromMsStyleProperty(StyleProperty& prop)
{
	char* str = new char[32];
	const char* propName = prop.LookupName();

	switch (prop.header.typeID)
	{
	case IDENTIFIER::FILENAME:
	{
		wxIntProperty* p = new wxIntProperty(propName, *wxPGProperty::sm_wxPG_LABEL, prop.data.imagetype.imageID);
		p->SetClientData(const_cast<void*>(static_cast<const void*>(&prop)));
		return p;
	}
	case IDENTIFIER::ENUM:
	{
		wxPGChoices* cp = GetEnumsFromMsStyleProperty(prop);
		wxPGProperty* p;
		
		if (cp != nullptr)
		{
			p = new wxEnumProperty(propName, *wxPGProperty::sm_wxPG_LABEL, *cp, prop.data.enumtype.enumvalue);
			delete cp;
		}
		else
		{
			p = new wxIntProperty(propName, *wxPGProperty::sm_wxPG_LABEL, prop.data.enumtype.enumvalue);
		}

		p->SetClientData(&prop);
		return p;
	}
	case IDENTIFIER::SIZE:
	{
		wxIntProperty* p = new wxIntProperty(propName, *wxPGProperty::sm_wxPG_LABEL, prop.data.sizetype.size);
		p->SetClientData(&prop);
		return p;
	}
	case IDENTIFIER::COLOR:
	{
		wxColourProperty* p = new wxColourProperty(propName, *wxPGProperty::sm_wxPG_LABEL, wxColor(prop.data.colortype.r, prop.data.colortype.g, prop.data.colortype.b));
		p->SetClientData(&prop);
		return p;
	}
	case IDENTIFIER::INT:
	{
		wxIntProperty* p = new wxIntProperty(propName, *wxPGProperty::sm_wxPG_LABEL, prop.data.inttype.value);
		p->SetClientData(&prop);
		return p;
	}
	case IDENTIFIER::BOOL:
	{
		wxIntProperty* p = new wxIntProperty(propName, *wxPGProperty::sm_wxPG_LABEL, prop.data.booltype.boolvalue);
		p->SetClientData(&prop);
		return p;
	}
	case IDENTIFIER::MARGINS:
	{
		sprintf(str, "%d, %d, %d, %d", prop.data.margintype.left, prop.data.margintype.top, prop.data.margintype.right, prop.data.margintype.bottom);
		wxStringProperty* p = new wxStringProperty(propName, *wxPGProperty::sm_wxPG_LABEL, str);
		p->SetClientData(&prop);
		return p;
	}
	case IDENTIFIER::RECT:
	{
		sprintf(str, "%d, %d, %d, %d", prop.data.recttype.left, prop.data.recttype.top, prop.data.recttype.right, prop.data.recttype.bottom);
		wxStringProperty* p = new wxStringProperty(propName, *wxPGProperty::sm_wxPG_LABEL, str);
		p->SetClientData(&prop);
		return p;
	}
	case IDENTIFIER::POSITION:
	{
		sprintf(str, "%d, %d", prop.data.positiontype.x, prop.data.positiontype.y);
		wxStringProperty* p = new wxStringProperty(propName, *wxPGProperty::sm_wxPG_LABEL, str);
		p->SetClientData(&prop);
		return p;
	}
	case IDENTIFIER::FONT:
	{
		wxIntProperty* p = new wxIntProperty("FONT (ID)", *wxPGProperty::sm_wxPG_LABEL, prop.data.fonttype.fontID);
		p->SetClientData(&prop);
		return p;
	}
	case IDENTIFIER::STRING:
	{
		wxStringProperty* p = new wxStringProperty(propName, *wxPGProperty::sm_wxPG_LABEL, &prop.data.texttype.firstchar);
		p->SetClientData(&prop);
		return p;
	}
	case IDENTIFIER::INTLIST:
	{
		if (prop.data.intlist.numints >= 3)
		{
			sprintf(str, "%d, %d, %d, .. (%d)", prop.data.intlist.numints
				, *(&prop.data.intlist.firstint + 0)
				, *(&prop.data.intlist.firstint + 1)
				, *(&prop.data.intlist.firstint + 2));
		}
		else sprintf(str, "Len: %d, Values omitted", prop.data.intlist.numints);
		wxStringProperty* p = new wxStringProperty(propName, *wxPGProperty::sm_wxPG_LABEL, str);
		p->SetClientData(&prop);
		return p;
	}
	default:
		wxStringProperty* p = new wxStringProperty(propName, *wxPGProperty::sm_wxPG_LABEL, "VALUE");
		p->SetClientData(&prop);
		return p;
	}
}

wxPGChoices* GetEnumsFromMsStyleProperty(libmsstyle::StyleProperty& prop)
{
	wxPGChoices* choices = new wxPGChoices();

	libmsstyle::lookup::EnumList result = libmsstyle::lookup::FindEnums(prop.header.nameID);
	if (result.enums == nullptr || result.numEnums == 0)
		return nullptr;

	for (int i = 0; i < result.numEnums; ++i)
	{
		choices->Add(result.enums[i].value, result.enums[i].key);
	}

	return choices;
}

std::string WideToUTF8(const std::wstring& str)
{
	std::wstring_convert<std::codecvt_utf8_utf16<wchar_t>, wchar_t> convert;
	return convert.to_bytes(str);
}

std::wstring UTF8ToWide(const std::string& str)
{
	std::wstring_convert<std::codecvt_utf8_utf16<wchar_t>, wchar_t> convert;
	return convert.from_bytes(str);
}
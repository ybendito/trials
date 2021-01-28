#include "stdafx.h"
#include "CommonTypes.h"

class tTokenizerClient
{
public:
	virtual void OnNewToken(CString& sToken) = 0;
};

class tSerializer
{
public:
	tSerializer(tSerializer *_parent, CString _name, CString _key, bool _bSave)
	{
		parent = _parent;
		name = _name;
		key = _key;
		bSave = _bSave;
	}
	CString Name() const { return name; }
	virtual void Tokenize(tTokenizerClient& client) = 0;
	virtual void CollectToken(CString& s) = 0;
	virtual void WriteString(CString sKey, CString s) = 0;
	virtual void WriteInteger(CString sKey, int val) = 0;
	virtual CString ReadString(CString sKey, CString _default = _T("")) = 0;
	virtual int ReadInteger(CString sKey, int _default = 0) = 0;
	virtual void _WriteString(CString sSection, CString sKey, CString s) = 0;
	virtual void _WriteInteger(CString sSection, CString sKey, int val) = 0;
	virtual CString _ReadString(CString sSection, CString sKey, CString _default) = 0;
	virtual int _ReadInteger(CString sSection, CString sKey, int _default) = 0;
protected:
	tSerializer *parent;
	CString name;
	CString key;
	bool bSave;
	CString value;
};

class tSimpleSerializer : public tSerializer
{
public:
	tSimpleSerializer(tSerializer *_parent, CString _name, CString _key, bool _bSave) : tSerializer(_parent, _name, _key, _bSave)
	{
		if (!parent)
		if (!bSave && !key.IsEmpty())	
		{
			value = _ReadString(name, key, _T(""));
		}
	}
	~tSimpleSerializer()
	{
		if (bSave && !value.IsEmpty() && !key.IsEmpty())
		{
			_WriteString(name, key, value);
		}
	}
	void WriteString(CString sKey, CString s);
	void WriteInteger(CString sKey, int val);
	CString ReadString(CString sKey, CString _default = _T(""));
	int ReadInteger(CString sKey, int _default = 0);
protected:
	virtual void Tokenize(tTokenizerClient& client)
	{
		int nextItem = 0;
		while (nextItem >= 0)
		{
			CString item = value.Tokenize(_T(","), nextItem);
			if (!item.IsEmpty())
			{
				client.OnNewToken(item);
			}
		}
	}
	virtual void CollectToken(CString& s)
	{
		if (!s.IsEmpty())
		{
			if (!value.IsEmpty()) value += TEXT(",");
			value += s;
		}
	}
	void _WriteString(CString sSection, CString sKey, CString s);
	void _WriteInteger(CString sSection, CString sKey, int val);
	CString _ReadString(CString sSection, CString sKey, CString _default);
	int _ReadInteger(CString sSection, CString sKey, int _default);
};

void tSimpleSerializer::WriteString(CString sKey, CString s)
{
	_WriteString(name, sKey, s);
}
void tSimpleSerializer::WriteInteger(CString sKey, int val)
{
	_WriteInteger(name, sKey, val);
}

CString tSimpleSerializer::ReadString(CString sKey, CString _default)
{
	return _ReadString(name, sKey, _default);
}

int tSimpleSerializer::ReadInteger(CString sKey, int _default)
{
	return _ReadInteger(name, sKey, _default);
}

void tSimpleSerializer::_WriteString(CString sSection, CString sKey, CString s)
{
	if (parent) 
		parent->_WriteString(sSection, sKey, s);
	else
		AfxGetApp()->WriteProfileString(sSection, sKey, s);	
}

void tSimpleSerializer::_WriteInteger(CString sSection, CString sKey, int val)
{
	if (parent) 
		parent->_WriteInteger(sSection, sKey, val);
	else
		AfxGetApp()->WriteProfileInt(sSection, sKey, val);	
}

CString tSimpleSerializer::_ReadString(CString sSection, CString sKey, CString _default)
{
	if (parent) 
		return parent->_ReadString(sSection, sKey, _default);
	else
		return AfxGetApp()->GetProfileString(sSection, sKey, _default);	
}

int tSimpleSerializer::_ReadInteger(CString sSection, CString sKey, int _default)
{
	if (parent) 
		return parent->_ReadInteger(sSection, sKey, _default);
	else
		return (int)AfxGetApp()->GetProfileInt(sSection, sKey, _default);	
}


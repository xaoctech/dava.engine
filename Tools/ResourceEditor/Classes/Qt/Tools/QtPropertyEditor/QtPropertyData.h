/*==================================================================================
    Copyright (c) 2008, binaryzebra
    All rights reserved.

    Redistribution and use in source and binary forms, with or without
    modification, are permitted provided that the following conditions are met:

    * Redistributions of source code must retain the above copyright
    notice, this list of conditions and the following disclaimer.
    * Redistributions in binary form must reproduce the above copyright
    notice, this list of conditions and the following disclaimer in the
    documentation and/or other materials provided with the distribution.
    * Neither the name of the binaryzebra nor the
    names of its contributors may be used to endorse or promote products
    derived from this software without specific prior written permission.

    THIS SOFTWARE IS PROVIDED BY THE binaryzebra AND CONTRIBUTORS "AS IS" AND
    ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
    WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
    DISCLAIMED. IN NO EVENT SHALL binaryzebra BE LIABLE FOR ANY
    DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
    (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
    LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND
    ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
    (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
    SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
=====================================================================================*/



#ifndef __QT_PROPERTY_DATA_H__
#define __QT_PROPERTY_DATA_H__

#include <QStyledItemDelegate>
#include <QHash>
#include <QIcon>
#include "QtPropertyModel.h"

// model class
class QtPropertyModel;

// Optional widget
struct QtPropertyOW
{
	QtPropertyOW() : widget(NULL), overlay(false), size(0, 0)
	{ }

	QtPropertyOW(QWidget *_widget, bool _overlay = false, QSize _size = QSize(16, 16))
		: widget(_widget), overlay(_overlay), size(_size)
	{ }

	QWidget *widget;
	QSize size;
	bool overlay;
};

QStandardItem;

// PropertyData class
class QtPropertyData : public QObject
{
	Q_OBJECT

	friend class QtPropertyModel;
	friend class QtPropertyItemDelegate;

public:
	enum ValueChangeReason
	{
		VALUE_SOURCE_CHANGED,
		VALUE_SET,
		VALUE_EDITED
	};

	QtPropertyData();
	QtPropertyData(const QVariant &value, Qt::ItemFlags flags = (Qt::ItemIsEnabled | Qt::ItemIsSelectable | Qt::ItemIsEditable));
	virtual ~QtPropertyData();

	QVariant data(int role) const;
	bool setData(const QVariant & value, int role);

	QString GetName() const;
	void SetName(const QString &name);

	QString GetPath() const;

	QVariant GetValue() const;
	void SetValue(const QVariant &value, ValueChangeReason reason = QtPropertyData::VALUE_SET);
	bool UpdateValue();

	QVariant GetAlias() const;

	QIcon GetIcon() const;
	void SetIcon(const QIcon &icon);

	QFont GetFont() const;
	void SetFont(const QFont &font);

	QBrush GetBackground() const;
	void SetBackground(const QBrush &brush);

	QBrush GetForeground() const;
	void SetForeground(const QBrush &brush);

	Qt::ItemFlags GetFlags() const;
	void SetFlags(Qt::ItemFlags flags);

	QVariant GetUserData() const;
	void SetUserData(const QVariant &data);

	// reset background/foreground/font settings
	void ResetStyle();

	void SetCheckable(bool checkable);
	bool IsCheckable() const;
	void SetChecked(bool checked);
	bool IsChecked() const;

	void SetEditable(bool editable);
	bool IsEditable() const;

	void SetEnabled(bool enabled);
	bool IsEnabled() const;

	QtPropertyModel* GetModel() const;

	// editor
	QWidget* CreateEditor(QWidget *parent, const QStyleOptionViewItem& option) const;
	bool EditorDone(QWidget *editor);
	bool SetEditorData(QWidget *editor);

	// childs
	QtPropertyData *Parent() const;
	void ChildAdd(const QString &key, QtPropertyData *data);
	void ChildAdd(const QString &key, const QVariant &value);
	int ChildCount() const;
	QtPropertyData* ChildGet(int i) const;
	QtPropertyData* ChildGet(const QString &key) const;
	int ChildIndex(QtPropertyData *data) const;
	void ChildRemove(QtPropertyData *data);
	void ChildRemove(const QString &key);
	void ChildRemove(int i);
	void ChildRemoveAll();

	// Optional widgets
	int GetOWCount() const;
	const QtPropertyOW* GetOW(int index = 0);
	void AddOW(const QtPropertyOW &ow);
	void RemOW(int index);
	void RemOW(QWidget *widget);

	// edit command
	virtual void* CreateLastCommand() const;

protected:
	mutable QVariant curValue;
	QVariant userData;

	QString curName;
	Qt::ItemFlags curFlags;

	QMap<int, QVariant> style;
	bool updatingValue;

	QtPropertyModel *model;
	QtPropertyData *parent;

	QList<QString> childrenNames;
	QList<QtPropertyData*> childrenData;

	QVector<QtPropertyOW> optionalWidgets;
	QWidget *optionalWidgetViewport;

	void EmitDataChanged(ValueChangeReason reason);

	virtual void UpdateUp();
	virtual void UpdateDown();

	// Functions should be re-implemented by sub-class
	virtual QVariant GetValueInternal() const;
	virtual QVariant GetValueAlias() const;
	virtual void SetValueInternal(const QVariant &value);
	virtual bool UpdateValueInternal();
	virtual QWidget* CreateEditorInternal(QWidget *parent, const QStyleOptionViewItem& option) const;
	virtual bool EditorDoneInternal(QWidget *editor);
	virtual bool SetEditorDataInternal(QWidget *editor);

	// viewport, where optional widgets (OW) should be drawn
	QWidget* GetOWViewport() const;
	void SetOWViewport(QWidget *viewport);
};

#endif // __QT_PROPERTY_DATA_H__

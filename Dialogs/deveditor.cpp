/**
  *********************************************************************************************************************************************************
  @file     :deveditor.cpp
  @brief    :Functions for device editor class (send data commands)
  *********************************************************************************************************************************************************
  ESPlot allows real-time communication between an embedded system and a computer offering signal processing and plotting capabilities and it relies on
  hardware graphics acceleration for systems disposing of OpenGL-compatible graphic units. More info at www.uni-saarland.de/lehrstuhl/nienhaus/esplot.
 
  Copyright (C) Universität des Saarlandes 2020. Authors: Emanuele Grasso and Niklas König.
 
  The Software and the associated materials have been developed at the Universität des Saarlandes (hereinafter "UdS").
  Any copyright or patent right is owned by and proprietary material of the UdS hereinafter the “Licensor”.
 
  This program is free software: you can redistribute it and/or modify
  it under the terms of the GNU Affero General Public License as
  published by the Free Software Foundation, either version 3 of the
  License, or any later version.
 
  This program is distributed in the hope that it will be useful,
  but WITHOUT ANY WARRANTY; without even the implied warranty of
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
  GNU Affero General Public License for more details.
 
  You should have received a copy of the GNU Affero General Public License
  along with this program. If not, see <https://www.gnu.org/licenses/>.
 
  This Agreement shall be governed by the laws of the Federal Republic of Germany except for the UN Sales Convention and the German rules of conflict of law.
 
  Commercial licensing opportunities
  For commercial uses of the Software beyond the conditions applied by AGPL 3.0 please contact the Licensor sending an email to patentverwertungsagentur@uni-saarland.de
  *********************************************************************************************************************************************************
  */
 
 #include "deveditor.h"

devEditor::devEditor()
{
    activeRowIndex = -1;
    Sig_Pool.clear();

    dataModel = new QStandardItemModel;
    dataView = new QTableView;

    setMinimumSize(150, 50);

    prepareViewModel();

    prepareWindow();

    resize(sizeHint());
}

devEditor::~devEditor()
{
    delete dataModel;
    delete dataView;
}

void devEditor::Add_Signal(QString name, int index, int initialValue)
{
    QList<QStandardItem*> newrow;
    devEditType element;

    element.sig_name = name;
    element.index = index;
    element.value = initialValue;

    Sig_Pool.append(element);

    newrow.append(new QStandardItem(name));
    newrow.append(new QStandardItem(QString::number(initialValue)));
    newrow.append(new QStandardItem("0x" + get_Hex(initialValue)));
    newrow.append(new QStandardItem("0b" + get_Bin(initialValue)));

    dataModel->appendRow(newrow);
}

QVector<int> devEditor::get_Data()
{
    QVector<int> data;

    data.resize(Sig_Pool.count());
    for (int i = 0; i < Sig_Pool.count(); i++)
        data[i] = Sig_Pool.at(i).value;

    return data;
}

QString devEditor::get_Command()
{
    return command_string;
}

void devEditor::itemClicked(const QModelIndex &index)
{
    if (index.isValid())
    {
        activeRowIndex = index.row();
        QModelIndex idx = dataModel->index(activeRowIndex, 0);
        QModelIndex index = dataModel->index(activeRowIndex, 1);

        nameLBL->setText(dataModel->data(idx).toString() + "   ");
        viewEntries(dataModel->data(index).toInt());
    }
}

void devEditor::updateCmdTxt()
{
    command_string = txtCommandLE->text();

    if (sendCB->isChecked() == true)
        emit dataReady();
}

void devEditor::updateFromHex()
{
    if (activeRowIndex == -1)
        return;

    int value;
    QString hex;
    bool ok;

    hex = hexEntry1LE->text() + hexEntry2LE->text() + hexEntry3LE->text() + hexEntry4LE->text();

    value = static_cast<int>(hex.toUInt(&ok, 16));

    viewEntries(value);
    updateDataModel();

    if (sendCB->isChecked() == true)
        emit dataReady();
}

void devEditor::updateFromBin()
{
    if (activeRowIndex == -1)
        return;

    int value;
    QString bin;
    bool ok;

    bin = binEntry1LE->text() + binEntry2LE->text() + binEntry3LE->text() + binEntry4LE->text();
    bin+= binEntry5LE->text() + binEntry6LE->text() + binEntry7LE->text() + binEntry8LE->text();

    value = static_cast<int>(bin.toUInt(&ok, 2));

    viewEntries(value);
    updateDataModel();

    if (sendCB->isChecked() == true)
        emit dataReady();
}

void devEditor::updateFromDec()
{
    if (activeRowIndex == -1)
        return;

    int value;

    value = decEntryLE->text().toInt();

    viewEntries(value);
    updateDataModel();

    if (sendCB->isChecked() == true)
        emit dataReady();
}

void devEditor::sendPushed()
{
    emit dataReady();
}

void devEditor::prepareViewModel()
{
    QStringList headers;

    dataModel->insertColumns(0, 4);

    headers.append("Variable Name");
    headers.append("Decimal Value");
    headers.append("Hexadecimal Expression");
    headers.append("Binary Expression");

    dataModel->setHorizontalHeaderLabels(headers);

    dataView->setModel(dataModel);

    dataView->setEditTriggers(QAbstractItemView::NoEditTriggers);

    connect(dataView, SIGNAL(clicked(const QModelIndex &)), this, SLOT(itemClicked(const QModelIndex &)));

}

void devEditor::prepareWindow()
{
    QRegularExpression hexVal("^[0-9A-F]{2}$", QRegularExpression::CaseInsensitiveOption);
    QRegularExpression binVal("^[0-1]{4}$");
    QLabel *dataLab = new QLabel("Variables list:");

    txtCommandLE = new QLineEdit;
    txtCommandLE->setMaxLength(16);  //Maximum 16 characters
    connect(txtCommandLE, &QLineEdit::editingFinished, this, &devEditor::updateCmdTxt);

    nameLBL = new QLabel("Variable name   ");

    decEntryLE = new QLineEdit;
    decEntryLE->setValidator(new QIntValidator); decEntryLE->setMaximumWidth(100);
    connect(decEntryLE, &QLineEdit::editingFinished, this, &devEditor::updateFromDec);

    hexEntry1LE = new QLineEdit;
    hexEntry1LE->setValidator(new QRegularExpressionValidator(hexVal)); hexEntry1LE->setMaximumWidth(25);
    connect(hexEntry1LE, &QLineEdit::editingFinished, this, &devEditor::updateFromHex);
    hexEntry2LE = new QLineEdit;
    hexEntry2LE->setValidator(new QRegularExpressionValidator(hexVal)); hexEntry2LE->setMaximumWidth(25);
    connect(hexEntry2LE, &QLineEdit::editingFinished, this, &devEditor::updateFromHex);
    hexEntry3LE = new QLineEdit;
    hexEntry3LE->setValidator(new QRegularExpressionValidator(hexVal)); hexEntry3LE->setMaximumWidth(25);
    connect(hexEntry3LE, &QLineEdit::editingFinished, this, &devEditor::updateFromHex);
    hexEntry4LE = new QLineEdit;
    hexEntry4LE->setValidator(new QRegularExpressionValidator(hexVal)); hexEntry4LE->setMaximumWidth(25);
    connect(hexEntry4LE, &QLineEdit::editingFinished, this, &devEditor::updateFromHex);

    binEntry1LE = new QLineEdit;
    binEntry1LE->setValidator(new QRegularExpressionValidator(binVal)); binEntry1LE->setMaximumWidth(50);
    connect(binEntry1LE, &QLineEdit::editingFinished, this, &devEditor::updateFromBin);
    binEntry2LE = new QLineEdit;
    binEntry2LE->setValidator(new QRegularExpressionValidator(binVal)); binEntry2LE->setMaximumWidth(50);
    connect(binEntry2LE, &QLineEdit::editingFinished, this, &devEditor::updateFromBin);
    binEntry3LE = new QLineEdit;
    binEntry3LE->setValidator(new QRegularExpressionValidator(binVal)); binEntry3LE->setMaximumWidth(50);
    connect(binEntry3LE, &QLineEdit::editingFinished, this, &devEditor::updateFromBin);
    binEntry4LE = new QLineEdit;
    binEntry4LE->setValidator(new QRegularExpressionValidator(binVal)); binEntry4LE->setMaximumWidth(50);
    connect(binEntry4LE, &QLineEdit::editingFinished, this, &devEditor::updateFromBin);
    binEntry5LE = new QLineEdit;
    binEntry5LE->setValidator(new QRegularExpressionValidator(binVal)); binEntry5LE->setMaximumWidth(50);
    connect(binEntry5LE, &QLineEdit::editingFinished, this, &devEditor::updateFromBin);
    binEntry6LE = new QLineEdit;
    binEntry6LE->setValidator(new QRegularExpressionValidator(binVal)); binEntry6LE->setMaximumWidth(50);
    connect(binEntry6LE, &QLineEdit::editingFinished, this, &devEditor::updateFromBin);
    binEntry7LE = new QLineEdit;
    binEntry7LE->setValidator(new QRegularExpressionValidator(binVal)); binEntry7LE->setMaximumWidth(50);
    connect(binEntry7LE, &QLineEdit::editingFinished, this, &devEditor::updateFromBin);
    binEntry8LE = new QLineEdit;
    binEntry8LE->setValidator(new QRegularExpressionValidator(binVal)); binEntry8LE->setMaximumWidth(50);
    connect(binEntry8LE, &QLineEdit::editingFinished, this, &devEditor::updateFromBin);

    sendCB = new QCheckBox;
    sendCB->setCheckable(true);
    sendCB->setChecked(false);
    sendCB->setText("Send on ENTER");

    sendBut = new QPushButton;
    sendBut->setText("Send");
    connect(sendBut, &QPushButton::clicked, this, &devEditor::sendPushed);

    QHBoxLayout *hor1Lay = new QHBoxLayout;
    QHBoxLayout *hor2Lay = new QHBoxLayout;

    QLabel *lab;

    hor1Lay->addWidget(nameLBL);
    lab = new QLabel("Decimal:"); lab->setAlignment(Qt::AlignCenter | Qt::AlignRight);
    hor1Lay->addWidget(lab);
    hor1Lay->addWidget(decEntryLE);
    lab = new QLabel("Hexadecimal:"); lab->setAlignment(Qt::AlignCenter | Qt::AlignRight);
    hor1Lay->addWidget(lab);
    hor1Lay->addWidget(hexEntry1LE); hor1Lay->addWidget(hexEntry2LE); hor1Lay->addWidget(hexEntry3LE); hor1Lay->addWidget(hexEntry4LE);
    hor1Lay->addStretch();

    lab = new QLabel("Binary:"); lab->setAlignment(Qt::AlignCenter | Qt::AlignRight);
    hor2Lay->addWidget(lab);
    hor2Lay->addWidget(binEntry1LE); hor2Lay->addWidget(binEntry2LE); hor2Lay->addWidget(binEntry3LE); hor2Lay->addWidget(binEntry4LE);
    hor2Lay->addWidget(binEntry5LE); hor2Lay->addWidget(binEntry6LE); hor2Lay->addWidget(binEntry7LE); hor2Lay->addWidget(binEntry8LE);
    hor2Lay->addWidget(sendBut);
    hor2Lay->addStretch();

    hor1Lay->setSizeConstraint(QLayout::SetFixedSize);
    hor2Lay->setSizeConstraint(QLayout::SetFixedSize);

    QVBoxLayout *layout;
    layout = new QVBoxLayout;
    layout->addWidget(new QLabel("Command string (max 16 characters):"));
    layout->addWidget(txtCommandLE);
    layout->addWidget(dataLab);
    layout->addWidget(dataView);
    layout->addLayout(hor1Lay);
    layout->addLayout(hor2Lay);
    layout->addWidget(sendCB);

    setLayout(layout);
}

QString devEditor::get_Hex(int value)
{
    int i;
    QString s;
    int L;

    s = QString::number(value, 16).toUpper();
    L = s.length();

    if (L < 8)  //we need to pad zeros at the beginning
        for (i = 0; i < 8 - L; i++)
            s.insert(0, '0');
    else
        s = s.remove(0, L - 8);

    return s;
}

QString devEditor::get_Bin(int value)
{
    int i;
    QString s;
    int L;

    s = QString::number(value, 2).toUpper();
    L = s.length();

    if (L < 32)  //we need to pad zeros at the beginning
        for (i = 0; i < 32 - L; i++)
            s.insert(0, '0');
    else
        s = s.remove(0, L - 32);

    return s;
}

void devEditor::viewEntries(int value)
{
    if ((activeRowIndex == -1) || (activeRowIndex >= Sig_Pool.count()))
        return;

    QString hex, bin, dec;

    dec = QString::number(value);
    hex = get_Hex(value);
    bin = get_Bin(value);

    decEntryLE->setText(dec);

    hexEntry1LE->setText(hex.mid(0, 2));
    hexEntry2LE->setText(hex.mid(2, 2));
    hexEntry3LE->setText(hex.mid(4, 2));
    hexEntry4LE->setText(hex.mid(6, 2));

    binEntry1LE->setText(bin.mid(0, 4));
    binEntry2LE->setText(bin.mid(4, 4));
    binEntry3LE->setText(bin.mid(8, 4));
    binEntry4LE->setText(bin.mid(12, 4));
    binEntry5LE->setText(bin.mid(16, 4));
    binEntry6LE->setText(bin.mid(20, 4));
    binEntry7LE->setText(bin.mid(24, 4));
    binEntry8LE->setText(bin.mid(28, 4));
}

void devEditor::updateEntries()
{
    if ((activeRowIndex == -1) || (activeRowIndex >= Sig_Pool.count()))
        return;

    QString hex, bin, dec;

    dec = decEntryLE->text();
    hex = "0x" + hexEntry1LE->text() + hexEntry2LE->text() + hexEntry3LE->text() + hexEntry4LE->text();
    bin = "0b" + binEntry1LE->text() + binEntry2LE->text() + binEntry3LE->text() + binEntry4LE->text();
    bin += binEntry5LE->text() + binEntry6LE->text() + binEntry7LE->text() + binEntry8LE->text();
}

void devEditor::updateDataModel()
{
    int value;

    value = decEntryLE->text().toInt();

    Sig_Pool[activeRowIndex].value = value;

    dataModel->setItem(activeRowIndex, 1, new QStandardItem(decEntryLE->text()));
    dataModel->setItem(activeRowIndex, 2, new QStandardItem("0x" + get_Hex(value)));
    dataModel->setItem(activeRowIndex, 3, new QStandardItem("0b" + get_Bin(value)));
}

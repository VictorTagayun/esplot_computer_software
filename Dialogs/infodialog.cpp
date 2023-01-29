/**
  *********************************************************************************************************************************************************
  @file     :infodialog.cpp
  @brief    :Functions for Info Dialog class
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

#include "infodialog.h"

infoDialog::infoDialog()
{
    QFontDatabase::addApplicationFont(":/fonts/3rdparty/Fonts/Quattrocento/Quattrocento-Regular.ttf");

    QFont font = QFont("Quattrocento", 40, 1);

    headerLabel = new QLabel("ES Plot");
    headerLabel->setFont(font);

    logo.load(":/Icons/Icons/EsPlotLogo.png");
    logo =  logo.scaled(250, 250, Qt::KeepAspectRatio);
    logoLabel = new QLabel(this);
    logoLabel->setPixmap(logo);
    logoLabel->setBackgroundRole(QPalette::Base);
    logoLabel->setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Fixed);
    logoLabel->setScaledContents(true);
    logoLabel->resize(logo.size());

    vertLay = new QVBoxLayout;
    horLay = new QHBoxLayout;

    vertLay->addWidget(headerLabel);
    vertLay->addWidget(logoLabel);

    vertLay->setAlignment(headerLabel, Qt::AlignHCenter);
    vertLay->setAlignment(logoLabel, Qt::AlignHCenter);

    tabWid = new QTabWidget(this);
    tabWid->addTab(new InfoTab(this), "Informations");
    tabWid->addTab(new AuthorTab(this), "Authors");
    tabWid->addTab(new AckTab(this), "Acknowledgments");


    horLay->addLayout(vertLay);
    horLay->addWidget(tabWid);

    setLayout(horLay);

    this->setFixedSize(1200,400);
}

InfoTab::InfoTab(QWidget *parent) : QWidget(parent)
{
    QString text;
    QTextBrowser *infoLabel = new QTextBrowser(parent);

    QFile f(":/info/Info/Info.txt");
    f.open(QFile::ReadOnly | QFile::Text);
    QTextStream in(&f);
    in.setCodec("ISO 8859-1");
    text = in.readAll();
    infoLabel->setText(text);
    infoLabel->setFont(QFont("Quattrocento", 12, 1));
    f.close();

    QVBoxLayout *lay = new QVBoxLayout(parent);
    lay->addWidget(infoLabel);
    setLayout(lay);

}

AuthorTab::AuthorTab(QWidget *parent) : QWidget(parent)
{
    QString text;
    QTextBrowser *infoLabel = new QTextBrowser(parent);

    QFile f(":/info/Info/Authors.txt");
    f.open(QFile::ReadOnly | QFile::Text);
    QTextStream in(&f);
    in.setCodec("ISO 8859-1");
    text = in.readAll();
    infoLabel->setText(text);
    infoLabel->setFont(QFont("Quattrocento", 12, 1));
    f.close();

    infoLabel->setText(text);

    QVBoxLayout *lay = new QVBoxLayout(parent);
    lay->addWidget(infoLabel);
    setLayout(lay);
}

AckTab::AckTab(QWidget *parent) : QWidget(parent)
{
    QString text;
    QTextBrowser *infoLabel = new QTextBrowser(parent);

    QFile f(":/info/Info/Acknowledgment.txt");
    f.open(QFile::ReadOnly | QFile::Text);
    QTextStream in(&f);
    in.setCodec("ISO 8859-1");
    text = in.readAll();
    infoLabel->setText(text);
    infoLabel->setFont(QFont("Quattrocento", 12, 1));
    f.close();


    infoLabel->setHtml(text);
    infoLabel->setOpenExternalLinks(true);

    latLabel = new QLabel(this);
    latLogo.load(":/Icons/Icons/LATlogo.jpg");
    latLogo = latLogo.scaledToHeight(100, Qt::TransformationMode::SmoothTransformation);
    latLabel->setPixmap(latLogo);
    latLabel->setBackgroundRole(QPalette::Base);
    latLabel->setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Fixed);
    latLabel->setScaledContents(true);
    latLabel->resize(latLogo.size());

    suLogo.load(":/Icons/Icons/SUlogo.jpg");
    suLogo =  suLogo.scaledToHeight(100, Qt::TransformationMode::SmoothTransformation);
    suLabel = new QLabel(this);
    suLabel->setPixmap(suLogo);
    suLabel->setBackgroundRole(QPalette::Base);
    suLabel->setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Fixed);
    suLabel->setScaledContents(true);
    suLabel->resize(suLogo.size());

    QLabel *support = new QLabel("This software is supported by the\nLaboratory of Actuation Technology\nof the Saarland University");
    support->setFont(QFont("Quattrocento", 12, 1));
    QHBoxLayout *latH = new QHBoxLayout;
    latH->addWidget(latLabel);
    latH->addStretch();
    latH->addWidget(support);
    latH->addStretch();
    latH->addWidget(suLabel);

    QVBoxLayout *lay = new QVBoxLayout;
    lay->addWidget(infoLabel);
    lay->addLayout(latH);
    setLayout(lay);
}

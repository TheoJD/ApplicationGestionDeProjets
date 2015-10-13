#include "menuprojet.h"

/**
 * \file menuprojet.cpp
 * \brief Défini les méthodes de menuprojet.h
 */

/**********************Menu Projet ************************************/

/**********************************************************************/
/*                  Onglet Creer nouveau projet                       */
/**********************************************************************/

CreationProjet::CreationProjet(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::CreationProjet)
{
    ui->setupUi(this);
    this->setModal(true);
    ui->pushButtonOK->setEnabled(false);
    ui->dateEditDispo->setDate(QDate::currentDate());
    ui->dateEditEch->setDate(QDate::currentDate());

    QObject::connect(ui->lineEdit, SIGNAL(textChanged(QString)),this,SLOT(activerOK()));
    QObject::connect(ui->pushButtonAnnuler, SIGNAL(clicked()),this,SLOT(close()));
    QObject::connect(ui->pushButtonOK, SIGNAL(clicked()),this,SLOT(save()));
    QObject::connect(ui->pushButtonOK, SIGNAL(clicked()),parent,SLOT(updateTree()));
    QObject::connect(this, SIGNAL(finished(int)),parent,SLOT(updateTree())); //permet d'actualiser l'arbre après la fermeture de la fenetre

}

void CreationProjet::activerOK(){
    if((ui->lineEdit->text().isEmpty()))
        ui->pushButtonOK->setEnabled(false);
    else
        ui->pushButtonOK->setEnabled(true);
}

void CreationProjet::save(){
    try{
        //creation du projet avec les infos
        ProjetManager::getInstance().ajouterProjet(ui->lineEdit->text(), ui->dateEditDispo->date(), ui->dateEditEch->date());
    }catch(CalendarException e){QMessageBox::warning(this, "Erreur", e.getInfo()); return;}

    QMessageBox::information(this, "Reussite", "Le projet \""+ui->lineEdit->text()+"\" a ete ajoute");

    //on ferme la fenetre de creation
    this->close();
}

CreationProjet::~CreationProjet() { delete ui; }

/**************************************************************************************************************************************/

/**********************************************************************/
/*                          Supprimmer Projet                         */
/**********************************************************************/

SuppressionProjet::SuppressionProjet(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::SuppressionProjet)
{
    ui->setupUi(this);
    remplirComboBoxProjet();

    QObject::connect(ui->pushButtonValider, SIGNAL(clicked()),this,SLOT(supprimmerProjet()));
    QObject::connect(ui->pushButtonValider, SIGNAL(clicked()),parent,SLOT(updateTree()));
    QObject::connect(ui->pushButtonAnnuler, SIGNAL(clicked()),this,SLOT(close()));
    QObject::connect(this, SIGNAL(finished(int)),parent,SLOT(updateTree())); //permet d'actualiser l'arbre après la fermeture de la fenetre

}

SuppressionProjet::~SuppressionProjet() { delete ui; }

void SuppressionProjet::remplirComboBoxProjet() {
    ui->comboBoxProjet->clear();
    ui->comboBoxProjet->addItem(" ");
    for(QMap<QString, Projet*>::const_iterator it= ProjetManager::getInstance().getProjets().cbegin();it!=ProjetManager::getInstance().getProjets().cend(); it++){
        ui->comboBoxProjet->addItem((*it)->getNom());
    }
}

void SuppressionProjet::supprimmerProjet() {
    try{
        if (ui->comboBoxProjet->currentText()!=QString(" ")) {
            Projet* projet=ProjetManager::getInstance().trouverProjet(ui->comboBoxProjet->currentText());
            ProjetManager::getInstance().removeProjet(projet);
            MessageValidation mv(QString("Succès"), QString("Le projet "+ui->comboBoxProjet->currentText()+" a été supprimé."));
            int rep = mv.exec();
            if (rep == QMessageBox::No)
                close();
            else { remplirComboBoxProjet(); }

        }
        else
            throw CalendarException("Veuillez séléctionner un projet");
    }
    catch(CalendarException e){
        QMessageBox::warning(this, "Erreur Suppression", e.getInfo());
    }
}

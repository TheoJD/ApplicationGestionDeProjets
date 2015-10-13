#include "mainwindow.h"

/**
 * \file mainwindow.cpp
 * \brief Défini les méthodes de mainwindow.h
 */

/**********************************************************************/
/*                        MainWindow                                       */
/**********************************************************************/


MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow)
{
    ui->setupUi(this);
    this->setTree(ui->treeView);
    ui->treeView->setEditTriggers(0);
    ui->stackedWidget->setCurrentIndex(0);


    QObject::connect(ui->treeView, SIGNAL(itemClicked(QTreeWidgetItem*, int)),this,SLOT(affichageDescription(QTreeWidgetItem*,int)));
    QObject::connect(ui->actionImporter, SIGNAL(triggered()),this,SLOT(chargerCalendrier()));
    QObject::connect(ui->pushButtonImport, SIGNAL(clicked()),this,SLOT(chargerCalendrier()));
    QObject::connect(ui->actionExporter_2, SIGNAL(triggered()),this,SLOT(sauverCalendrier()));
    QObject::connect(ui->actionCreerNouvProj, SIGNAL(triggered()),this,SLOT(creerProjet()));
    QObject::connect(ui->actionSupprimet_Projet, SIGNAL(triggered()),this,SLOT(supprimerProjet()));
    QObject::connect(ui->actionNouvelleTache, SIGNAL(triggered()),this,SLOT(creerTache()));
    QObject::connect(ui->actionAjouter_Pr_decende, SIGNAL(triggered()),this,SLOT(nouvellePrec()));
    QObject::connect(ui->actionAjouter_Sous_T_che, SIGNAL(triggered()),this,SLOT(ajouterSousTache()));
    QObject::connect(ui->actionSupprimer_T_che, SIGNAL(triggered()),this,SLOT(supprimerTache()));
    QObject::connect(ui->actionSupprimer_2, SIGNAL(triggered()),this,SLOT(supprimerPrecedence()));
    QObject::connect(ui->actionSupprimer, SIGNAL(triggered()),this,SLOT(suppressionSousTache()));
    QObject::connect(ui->actionProgAct, SIGNAL(triggered()),this,SLOT(programAct()));
    QObject::connect(ui->actionProgrammation_d_une_t_che, SIGNAL(triggered()),this,SLOT(programTache()));
    QObject::connect(ui->actionD_programmer_T_che, SIGNAL(triggered()),this,SLOT(deprogrammerTache()));
    QObject::connect(ui->actionD_programmer_Activit, SIGNAL(triggered()),this,SLOT(deprogrammerActivite()));
    QObject::connect(ui->actionAffichage_3, SIGNAL(triggered()),this,SLOT(afficheEmploi()));

}

void MainWindow::setTree(QTreeWidget* arbre){
    arbre->clear();
    for(QMap<QString, Projet*>::const_iterator it= ProjetManager::getInstance().getProjets().cbegin();it!=ProjetManager::getInstance().getProjets().cend(); it++){
       QTreeWidgetItem* item = new QTreeWidgetItem;
       item->setText(0,(*it)->getNom());
       QFont f = QFont();
       f.setBold(true);
       f.setUnderline(false);
       item->setFont(0,f);
       arbre->addTopLevelItem(item);

        for(QMap<QString, Tache*>::const_iterator it2 = (*it)->getTaches().cbegin();  it2!=(*it)->getTaches().cend(); it2++){//parcours des taches de chaque projet
            if ((*it2)->getTacheMere()==0) {
                TreeWidgetItem* item2 = new TreeWidgetItem;
                item2->setText(0,(*it2)->getId());//création du titre de la tache
                if ((*it2)->isComposite())
                    item2->setSousTaches(static_cast<const TacheComposite*>(*it2));
                item->addChild(item2); //ajout de item2 au projet
            }
        }
    }
}

void TreeWidgetItem::setSousTaches(const TacheComposite *t) {
    for(QMap<QString, Tache*>::const_iterator it = t->getSousTaches().cbegin();  it!=t->getSousTaches().cend(); it++){//parcours des taches de chaque projet
        TreeWidgetItem* item = new TreeWidgetItem;
        item->setText(0, (*it)->getId());//création du titre de la tache
        if ((*it)->isComposite())
            item->setSousTaches(static_cast<const TacheComposite*>(*it));
        this->addChild(item); //ajout de item à la tache composite
    }
}

/**********************************************************************/
/*                        Slots                                       */
/**********************************************************************/

void MainWindow::updateTree(){ //slot qui actualise l'arbre principal
    setTree(ui->treeView);
}

void MainWindow::affichageDescription(QTreeWidgetItem * item, int column) {
    Projet* p=ProjetManager::getInstance().trouverProjet(item->text(column));
    if (p!=0) {
        ui->stackedWidget->setCurrentIndex(1);
        ui->labelNomProjet->setText(p->getNom());
        ui->labelDispoProjet->setText(p->getDateDisponibilite().toString("dddd d MMMM yyyy"));
        ui->labelEcheProjet->setText(p->getDateEcheance().toString("dddd d MMMM yyyy"));
	}
    else {
        Tache* t=ProjetManager::getInstance().trouverTache(item->text(column));
        if (t!=0) {
            ui->stackedWidget->setCurrentIndex(2);
            ui->labelIdTache->setText(t->getId());
            ui->labelDispoTache->setText(t->getDateDisponibilite().toString("dddd d MMMM yyyy"));
            ui->labelEcheTache->setText(t->getDateEcheance().toString("dddd d MMMM yyyy"));
            ui->listPred->clear();
            ui->listPred->setSelectionMode(QAbstractItemView::NoSelection);
            for (QMap<QString,Tache*>::const_iterator it=t->getPred().begin(); it!=t->getPred().end(); ++it) {
                ui->listPred->addItem((*it)->getId());
            }
            if (t->isUnitaire()) {
                ui->stackedWidget_2->setCurrentIndex(0);
                ui->labelTypeTache->setText(QString("Unitaire"));
                if (static_cast<TacheUnitaire*>(t)->isPreemptive())
                    ui->labelPreemptTache->setText(QString("Oui"));
                else
                    ui->labelPreemptTache->setText(QString("Non"));
                ui->labelDureeTache->setText(t->getDuree().stringDuree());
            }
            else {
                ui->stackedWidget_2->setCurrentIndex(1);
                ui->labelTypeTache->setText(QString("Composite"));
            }
        }
        else
            throw CalendarException("Aucun projet ni tâche sélectionnés.");
    }
}

void MainWindow::chargerCalendrier() {

    try {
        if (file!="") {
        MessageSauvegarde ms(QString("Enregistrer"), QString("Vous allez changer de calendrier."));
        int rep = ms.exec();
        if (rep == QMessageBox::Yes)
            save(file);
        }
        file = QFileDialog::getOpenFileName();
        if (file!="")
            load(file);
        else throw CalendarException("Aucun calendrier chargé");
        updateTree();
    }
    catch (CalendarException e) {
         QMessageBox::information(this, "Erreur", e.getInfo());
    }
}

void MainWindow::sauverCalendrier() {
    try {
        if (file=="") {
            file = QFileDialog::getSaveFileName();
        }
        if (file!="")
            save(file);
        else throw CalendarException("Aucun calendrier sauvé");
        file = "";
        updateTree();
    }
    catch (CalendarException e) {
         QMessageBox::information(this, "Erreur", e.getInfo());
    }
}

void MainWindow::creerProjet(){
    CreationProjet* c = new CreationProjet(this);
    c->show();
}

void MainWindow::supprimerProjet(){
    SuppressionProjet* sp = new SuppressionProjet(this);
    sp->show();
}

void MainWindow::creerTache(){
    EditeurTache* ed = new EditeurTache(this);
    ed->show();
}


void MainWindow::nouvellePrec(){
    NouvellePrecedence* n = new NouvellePrecedence(this);
    n->show();
}

void MainWindow::ajouterSousTache(){
    AjoutSousTache* ast = new AjoutSousTache(this);
    ast->show();
}

void MainWindow::supprimerTache(){
    SuppressionTache* st = new SuppressionTache(this);
    st->show();
}

void MainWindow::supprimerPrecedence(){
    SuppressionPrecedence* sp = new SuppressionPrecedence(this);
    sp->show();
}

void MainWindow::suppressionSousTache(){
    SupprimerSousTache* sst = new SupprimerSousTache(this);
    sst->show();
}

void MainWindow::programAct(){
    ProgActivite* act = new ProgActivite(this);
    act->show();
}

void MainWindow::programTache(){
    ProgTache* pt = new ProgTache(this);
    pt->show();
}

void MainWindow::deprogrammerTache(){
    DeprogTache* dp = new DeprogTache(this);
    dp->show();
}

void MainWindow::deprogrammerActivite(){
    DeprogActivite* da = new DeprogActivite(this);
    da->show();
}

void MainWindow::afficheEmploi(){
    Semaine* sem = new Semaine(this);
    sem->show();
}


MainWindow::~MainWindow() {
    ProjetManager::libererInstance();
    ProgrammationManager::libererInstance();
    delete ui;
}

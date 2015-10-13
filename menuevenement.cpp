#include "menuevenement.h"

/**
 * \file menuevenement.cpp
 * \brief Défini les méthodes de menuevenement.h
 */

/**********************Menu Evenement ************************************/

/**********************************************************************/
/*            Onglet Programmation d'une activité                     */
/**********************************************************************/

ProgActivite::ProgActivite(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::ProgActivite)
{
    ui->setupUi(this);
    ui->dateTimeEdit->setDateTime(QDateTime::currentDateTime());
    ui->dateTimeEdit->setMinimumDateTime(QDateTime::currentDateTime());

    QObject::connect(ui->pushButtonAnnuler, SIGNAL(clicked()), this, SLOT(close()));
    QObject::connect(ui->pushButtonValider, SIGNAL(clicked()), this, SLOT(EnregistrerProg()));
}

void ProgActivite::EnregistrerProg(){
    if((ui->lineEditType->text()!=NULL) && (ui->lineEditDescription->text()!=NULL) && (ui->lineEditLieu->text()!=NULL) && (ui->lineEditNom->text()!=NULL)){
        Duree d = Duree(ui->timeEdit->time().hour(),ui->timeEdit->time().minute());
        try{
            ProgrammationManager::getInstance().ajouterProgrammationEvenement(ui->lineEditNom->text(), ui->lineEditType->text(), ui->lineEditDescription->text(), ui->lineEditLieu->text(), ui->dateTimeEdit->dateTime(), d);
            MessageValidation mv(QString("Succès"), QString("L'activité a été ajoutée."));
            int rep = mv.exec();
            if (rep == QMessageBox::No)
                close();
            else {
                //Si l'utilisateur veut ajouter une nouvelle activité, on réinitialise tous les champs
                ui->lineEditNom->clear();
                ui->lineEditType->clear();
                ui->lineEditDescription->clear();
                ui->lineEditLieu->clear();
                ui->dateTimeEdit->setDateTime(QDateTime::currentDateTime());
                ui->timeEdit->setTime(QTime(0, 0));}
        }
        catch(CalendarException e){ QMessageBox::warning(this, "Erreur", e.getInfo()); }
    }
    else
        QMessageBox::information(this, "Attention", "Veuillez remplir tous les champs");
}

ProgActivite::~ProgActivite() { delete ui; }


/**********************************************************************/
/*            Onglet Programmation d'une Tache                        */
/**********************************************************************/

ProgTache::ProgTache(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::ProgTache)
{
    ui->setupUi(this);
    ui->dateTimeEdit_2->setDateTime(QDateTime::currentDateTime());
    ui->dateTimeEdit_2->setMinimumDateTime(QDateTime::currentDateTime());

    ui->comboBoxProjet->addItem(QString(" "));
    for(QMap<QString, Projet*>::const_iterator it= ProjetManager::getInstance().getProjets().cbegin();it!=ProjetManager::getInstance().getProjets().cend(); it++){
        if ((*it)->getDateEcheance()>=QDate::currentDate())
            ui->comboBoxProjet->addItem((*it)->getNom());
    }

    QObject::connect(ui->comboBoxProjet, SIGNAL(activated(QString)), this, SLOT(remplirComboTache(QString)));
    QObject::connect(ui->pushButtonAnnuler_2, SIGNAL(clicked()), this, SLOT(close()));
    QObject::connect(ui->pushButtonValider_2, SIGNAL(clicked()), this, SLOT(EnregistrerProgTache()));
    QObject::connect(ui->comboBoxTache, SIGNAL(activated(QString)), this, SLOT(SetInfos(QString)));
}



void ProgTache::remplirComboTache(const QString& p){
    ui->comboBoxTache->clear();
    try{
        ui->comboBoxTache->addItem(" ");
        Projet* projet= ProjetManager::getInstance().trouverProjet(p);
        if(projet!=0){
            for (QMap<QString,Tache*>::const_iterator it=projet->getTaches().begin(); it!=projet->getTaches().end(); ++it) {
                if ((*it)->isUnitaire() && (*it)->getDateEcheance()>=QDate::currentDate() && static_cast<TacheUnitaire*>(*it)->dureeRestanteAProgrammer().getDureeEnMinutes()>0)
                    ui->comboBoxTache->addItem((*it)->getId());
            }
        }
    }
    catch(CalendarException e){
        QMessageBox::warning(this, "Erreur", e.getInfo());
    }
}

void ProgTache::SetInfos(const QString& t){
    if (t!=QString(" ")) {
        Tache* tache= ProjetManager::getInstance().trouverProjet(ui->comboBoxProjet->currentText())->trouverTache(t);
        ui->labelTitre_2->setText(tache->getTitre());
        ui->dateTimeEdit_2->setMaximumDateTime(QDateTime(tache->getDateEcheance().addDays(1))); //On ne peut programmer après l'échéance de la tâche
        if(tache->isUnitaire()){
            if(static_cast<TacheUnitaire*>(tache)->isPreemptive()) {
                ui->labelPreemptive_2->setText("oui");
                //L'utilisateur peut programmer la tâche en plusieurs fois, on plafonne simplement la durée maximale qui est la durée restante à programmer
                Duree dur=static_cast<TacheUnitaire*>(tache)->dureeRestanteAProgrammer();
                QTime t(dur.getHeure(),dur.getMinute());
                ui->timeEdit_2->setTime(t);
                ui->timeEdit_2->setMaximumTime(t);
            }
            else {
                ui->labelPreemptive_2->setText("non");
                //L'utilisateur ne pourra programmer la tâche qu'en une seule fois, donc on guide l'utilisateur en empêchant de programmer une durée autre
                Duree dur=static_cast<TacheUnitaire*>(tache)->getDuree();
                QTime t(dur.getHeure(),dur.getMinute());
                ui->timeEdit_2->setTime(t);
                ui->timeEdit_2->setMaximumTime(t);
                ui->timeEdit_2->setMinimumTime(t);
            }
            ui->labelDuree_2->setText(tache->getDuree().stringDuree());
        }
        ui->labelDispo_2->setText(QString::number(tache->getDateDisponibilite().day())+"/"+QString::number(tache->getDateDisponibilite().month())+"/"+QString::number(tache->getDateDisponibilite().year()));
        ui->labelEcheance_2->setText(QString::number(tache->getDateEcheance().day())+"/"+QString::number(tache->getDateEcheance().month())+"/"+QString::number(tache->getDateEcheance().year()));
    }
    else {
        //On a plus de tâche séléctionée, on réinitialise tous les champs.
        ui->labelDispo_2->clear();
        ui->labelEcheance_2->clear();
        ui->labelDuree_2->clear();
        ui->labelPreemptive_2->clear();
        ui->labelTitre_2->clear();
    }
}

void ProgTache::EnregistrerProgTache(){
    if(ProjetManager::getInstance().isTacheExistante(ui->comboBoxTache->currentText())){
        TacheUnitaire* t = dynamic_cast<TacheUnitaire*>(ProjetManager::getInstance().trouverProjet(ui->comboBoxProjet->currentText())->trouverTache(ui->comboBoxTache->currentText()));
        Duree d = Duree(ui->timeEdit_2->time().hour(),ui->timeEdit_2->time().minute());
        try {
            ProgrammationManager::getInstance().ajouterProgrammationTache(t, ui->dateTimeEdit_2->dateTime(), d);
            MessageValidation mv(QString("Succès"), QString("La programmation a été ajoutée."));
            int rep = mv.exec();
            if (rep == QMessageBox::No)
                close();
            else {
                //Si l'utilisateur veut programmer une nouvelle tâche
                remplirComboTache(ui->comboBoxProjet->currentText()); //On actualise les tâches disponibles à la programamtion
                ui->timeEdit_2->setTime(QTime(0,0)); //On réinitialise la durée de programmation
                SetInfos(QString(" ")); //On enlève les infos de la tâche précédemment programmée
            }
        }
        catch(CalendarException e){ QMessageBox::warning(this, "Erreur", e.getInfo()); }
    }
    else
        QMessageBox::warning(this, "Programmation", "Veuillez séléctionner une tâche");
}

ProgTache::~ProgTache() { delete ui; }


/**********************************************************************/
/*            Onglet Deprogrammation d'une Tache                      */
/**********************************************************************/

DeprogTache::DeprogTache(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::DeprogTache)
{
    ui->setupUi(this);

    ui->comboBoxProjet->addItem(" ");
    for(QMap<QString, Projet*>::const_iterator it= ProjetManager::getInstance().getProjets().cbegin();it!=ProjetManager::getInstance().getProjets().cend(); it++){
       ui->comboBoxProjet->addItem((*it)->getNom());
    }

    QObject::connect(ui->radioButtonPart, SIGNAL(clicked(bool)),this,SLOT(showPropPart(bool)));//affiche la case programamtions pour la déprogrammation partielle
    QObject::connect(ui->comboBoxTache, SIGNAL(activated(QString)),this,SLOT(showProgrammation(QString)));//remplit la comboxprogrammation
    QObject::connect(ui->radioButtonTot, SIGNAL(clicked(bool)),this,SLOT(showPropTot(bool)));
    QObject::connect(ui->comboBoxProjet, SIGNAL(activated(QString)), this, SLOT(remplirComboTache(QString)));
    QObject::connect(ui->pushButtonValider, SIGNAL(clicked()), this, SLOT(EnregistrerDeprogTache()));
    QObject::connect(ui->pushButtonAnnuler, SIGNAL(clicked()), this, SLOT(close()));
}

DeprogTache::~DeprogTache() { delete ui; }

void DeprogTache::showPropPart(bool b){//affiche la partie montrant les programmations
    if(b==true){
        ui->stackedWidget->setCurrentIndex(1);
    }
}

void DeprogTache::showPropTot(bool b){
    if(b==true){
        ui->stackedWidget->setCurrentIndex(0);
    }
}

void DeprogTache::showProgrammation(const QString& t){
    ui->listProgrammation->clear();
    try {
        if(t!=QString(" ")){
            TacheUnitaire* tache = dynamic_cast<TacheUnitaire*>(ProjetManager::getInstance().trouverTache(t));
            QMap<QDateTime,Programmation*> programmations=ProgrammationManager::getInstance().getProgrammations(tache);
            for (QMap<QDateTime,Programmation*>::const_iterator it=programmations.begin(); it!=programmations.end(); ++it) {
                ui->listProgrammation->addItem((*it)->getDateTime().toString("dddd d MMMM yyyy H:m"));
                //ui->comboBoxProgrammation->addItem((*it)->getDateTime().toString("ddd dd/MM/yyyy H:m"));
                //qDebug()<<(*it)->getDateTime();
            }
            ui->listProgrammation->setCurrentRow(0);
        }
    }
    catch(CalendarException e){
        QMessageBox::warning(this, "Erreur", e.getInfo());
    }
}

void DeprogTache::remplirComboTache(const QString& p){
    ui->comboBoxTache->clear();
    try{
        ui->comboBoxTache->addItem(" ");
        Projet* projet= ProjetManager::getInstance().trouverProjet(p);
        if(projet!=0){
            for (QMap<QString,Tache*>::const_iterator it=projet->getTaches().begin(); it!=projet->getTaches().end(); ++it) {
                if ((*it)->isUnitaire() && static_cast<TacheUnitaire*>(*it)->dureeRestanteAProgrammer()<(*it)->getDuree())
                    ui->comboBoxTache->addItem((*it)->getId());
            }
        }
        showProgrammation(QString(" "));
    }
    catch(CalendarException e){
        QMessageBox::warning(this, "Erreur", e.getInfo());
    }
}

void DeprogTache::EnregistrerDeprogTache(){
    if(ProjetManager::getInstance().isTacheExistante(ui->comboBoxTache->currentText())){
        TacheUnitaire* t = dynamic_cast<TacheUnitaire*>(ProjetManager::getInstance().trouverProjet(ui->comboBoxProjet->currentText())->trouverTache(ui->comboBoxTache->currentText()));
        try {
            if(ui->radioButtonTot->isChecked() || ui->radioButtonPart->isChecked()){
                MessageValidation confirmation(QString("Attention"), QString("Si des tâches sont précédées par "+ui->comboBoxTache->currentText()+" elles seront déprogrammées en cascade."));
                int sur = confirmation.exec();
                if (sur == QMessageBox::Yes) {
                    if (ui->radioButtonTot->isChecked()) {
                        ProgrammationManager::getInstance().deprogrammationTotale(t);
                        MessageValidation mv(QString("Succès"), QString("La Tâche "+ui->comboBoxTache->currentText()+" a été déprogrammée avec succès"));
                        int rep = mv.exec();

                        if (rep == QMessageBox::No)
                            close();
                        else {
                            //Si l'utilisateur veut déprogrammer une nouvelle programmation
                            remplirComboTache(ui->comboBoxProjet->currentText()); //On actualise les tâches disponibles à la programamtion
                            ui->radioButtonTot->setChecked(false);

                        }
                    }
                    else {
                        QString prog=ui->listProgrammation->currentItem()->text();
                        QDateTime dt=QDateTime::fromString(prog,"dddd d MMMM yyyy H:m").toLocalTime();
                        /*if (dt<QDateTime::currentDateTime())
                            throw CalendarException("Impossible d'annuler une programmation dans le passé");*/
                        ProgrammationManager::getInstance().deprogrammationPartielle(t,dt);
                        MessageValidation mv(QString("Succès"), QString("La programmation de la tâche "+ui->comboBoxTache->currentText()+" a été retirée avec succès"));
                        int rep = mv.exec();
                        if (rep == QMessageBox::No)
                            close();
                        else {
                            //Si l'utilisateur veut déprogrammer une nouvelle programmation
                            remplirComboTache(ui->comboBoxProjet->currentText()); //On actualise les tâches disponibles à la programamtion
                            ui->radioButtonPart->setChecked(false);
                        }
                    }
                }
            }
            else throw CalendarException("Choississez le type de déprogrammation");
        }
        catch(CalendarException e){ QMessageBox::warning(this, "Erreur", e.getInfo()); }
    }
    else
        QMessageBox::warning(this, "Programmation", "Veuillez séléctionner une tâche");
}

/**********************************************************************/
/*            Onglet Programmation d'une activité                     */
/**********************************************************************/

DeprogActivite::DeprogActivite(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::DeprogActivite)
{
    ui->setupUi(this);
    remplirComboBoxActivite();

    QObject::connect(ui->comboBoxActivite, SIGNAL(activated(QString)), this, SLOT(showProgrammation(QString)));
    QObject::connect(ui->pushButtonAnnuler, SIGNAL(clicked()), this, SLOT(close()));
    QObject::connect(ui->pushButtonValider, SIGNAL(clicked()), this, SLOT(EnregistrerDeprogActivite()));
}

void DeprogActivite::remplirComboBoxActivite() {
    ui->comboBoxActivite->clear();
    ui->comboBoxActivite->addItem(QString(" "));
    const QMap<QDateTime,Programmation*>& progs = ProgrammationManager::getInstance().getProgrammations();
    for (QMap<QDateTime,Programmation*>::const_iterator it=progs.begin(); it!=progs.end(); ++it) {
        if ((*it)->getEvenement()!=0) {
            ui->comboBoxActivite->addItem((*it)->getEvenement()->getNom());
        }
    }
}

void DeprogActivite::showProgrammation(const QString& name) {
    const Programmation* activite=ProgrammationManager::getInstance().trouverEvenement(name);
    if (activite!=0) {
        if (activite->getDuree().getMinute()<10) {
            ui->labelProgrammation->setText(activite->getDateTime().toString("dddd d MMMM yyyy H:m")+" | Durée : "+QString::number(activite->getDuree().getHeure())+"h0"+QString::number(activite->getDuree().getMinute()));
        }
        else {
            ui->labelProgrammation->setText(activite->getDateTime().toString("dddd d MMMM yyyy H:m")+" | Durée : "+QString::number(activite->getDuree().getHeure())+"h"+QString::number(activite->getDuree().getMinute()));
        }
    }
    else
        ui->labelProgrammation->clear();
}

void DeprogActivite::EnregistrerDeprogActivite(){
    const Programmation* activite=ProgrammationManager::getInstance().trouverEvenement(ui->comboBoxActivite->currentText());
    if (activite!=0){
        try{
            ProgrammationManager::getInstance().deprogrammationEvenement(activite->getDateTime());
            MessageValidation mv(QString("Succès"), QString("L'activité a été déprogrammée."));
            int rep = mv.exec();
            if (rep == QMessageBox::No)
                close();
            else {
                ui->labelProgrammation->clear();
                remplirComboBoxActivite();
            }
        }
        catch(CalendarException e){ QMessageBox::warning(this, "Erreur", e.getInfo()); }
    }
    else
        QMessageBox::information(this, "Attention", "Veuillez selectionner une activite");
}

DeprogActivite::~DeprogActivite() { delete ui; }

#include "menutache.h"

/**
 * \file menutache.cpp
 * \brief Défini les méthodes de menutache.h
 */

EditeurTache::EditeurTache(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::EditeurTache)
{
    ui->setupUi(this);
    this->setModal(true);
    ui->stackedWidget->setCurrentIndex(0);
    ui->dateEditDispo->setDate(QDate::currentDate());
    ui->dateEditEch->setDate(QDate::currentDate());

    ui->comboBoxProjet->addItem(" ");
    ui->comboBoxMere->addItem(" ");
    //boucle pour remplir la combobox proposant les projets
    for(QMap<QString, Projet*>::const_iterator it= ProjetManager::getInstance().getProjets().cbegin();it!=ProjetManager::getInstance().getProjets().cend(); it++){
        if ((*it)->getDateEcheance()>=QDate::currentDate())
            ui->comboBoxProjet->addItem((*it)->getNom());
    }

    QObject::connect(ui->radioButtonUnit, SIGNAL(clicked(bool)),this,SLOT(showPropUnit(bool)));//affiche la partie demandant les prop des taches unitaire
    QObject::connect(ui->radioButtonComp, SIGNAL(clicked(bool)),this,SLOT(showPropComp(bool)));//affiche la partie demandant les prop des taches composites
    QObject::connect(ui->comboBoxProjet, SIGNAL(currentIndexChanged(QString)), this, SLOT(remplirComboMere(QString))); //affiche les tâches mères possible en fonction du projet séléctionné
    QObject::connect(ui->comboBoxProjet, SIGNAL(currentIndexChanged(QString)), this, SLOT(setDates(QString)));//Par défaut, initialise les dates de dispo et d'échéance en fonction de celles du projet
    QObject::connect(ui->pushButtonAjouter, SIGNAL(clicked()), this, SLOT(choixPrec()));//affiche nouvelle fenetre pour choisir les taches de précédence
    QObject::connect(ui->pushButtonOK, SIGNAL(clicked()), this, SLOT(enregisterTache()));
    QObject::connect(ui->pushButtonOK, SIGNAL(clicked()),parent,SLOT(updateTree()));
    QObject::connect(ui->pushButtonAnnuler, SIGNAL(clicked()),this,SLOT(close()));
    QObject::connect(this, SIGNAL(finished(int)),parent,SLOT(updateTree()));
}

QMap<QString, Tache*> EditeurTache::predecesseurs ;

void EditeurTache::showPropUnit(bool b){//affiche la partie demandant les prop des taches unitaires
    if(b==true){
        ui->stackedWidget->setCurrentIndex(1);
        ui->timeEditDuree->setMinimumTime(QTime(0,0));
    }
}
void EditeurTache::showPropComp(bool b){//affiche la partie demandant les prop des taches composites
    if(b==true){
        ui->stackedWidget->setCurrentIndex(2);
    }
}

void EditeurTache::setDates(QString s) {
    if(s!=" "){
        ui->dateEditDispo->setMinimumDate(ProjetManager::getInstance().trouverProjet(s)->getDateDisponibilite());
        ui->dateEditDispo->setMaximumDate(ProjetManager::getInstance().trouverProjet(s)->getDateEcheance());
        ui->dateEditDispo->setDate(ProjetManager::getInstance().trouverProjet(s)->getDateDisponibilite());
        ui->dateEditEch->setMinimumDate(ProjetManager::getInstance().trouverProjet(s)->getDateDisponibilite());
        ui->dateEditEch->setMaximumDate(ProjetManager::getInstance().trouverProjet(s)->getDateEcheance());
        ui->dateEditEch->setDate(ProjetManager::getInstance().trouverProjet(s)->getDateEcheance());
    }
}

void EditeurTache::remplirComboMere(QString s){
    ui->comboBoxMere->clear();
    ui->comboBoxMere->addItem(" ");
    if(s!=" "){
        QMap<QString, Tache*> tachesComposites= ProjetManager::getInstance().trouverProjet(s)->getTachesComposites();
        if(!tachesComposites.isEmpty())
            {
           for(QMap<QString, Tache*>::const_iterator it= tachesComposites.cbegin();it!=tachesComposites.cend(); it++){
               ui->comboBoxMere->addItem((*it)->getId());
           }
        }
    }
}

void EditeurTache::choixPrec(){
    AjoutPrecedence* p = new AjoutPrecedence(this);
    p->show();
}

void EditeurTache::enregisterTache(){
    if(!(ProjetManager::getInstance().isProjetExistant(ui->comboBoxProjet->currentText()))){
        QMessageBox::warning(this, "Attention", "Veuilliez choisir un projet.");
    }
    else{
        QString id = ui->lineEditId->text();
        QString titre = ui->lineEditTitre->text();
        if((id==NULL) || (titre==NULL))
            QMessageBox::warning(this, "Attention", "Veuilliez donnez un identificateur et un titre.");
        else{
            QDate disp = ui->dateEditDispo->date();
            QDate ech = ui->dateEditEch->date();

            if(ui->radioButtonUnit->isChecked()){
                Duree d = Duree(ui->timeEditDuree->time().hour(),ui->timeEditDuree->time().minute());
                bool pre = ui->radioButtonOui->isChecked();

                if((d.getDureeEnMinutes()==0)|| (pre==false && ui->radioButtonNon->isChecked()==false))//test si les champs durée et preemptive sont bien renseignés
                    QMessageBox::warning(this, "Attention", "Veuilliez renseigner la durée de la tache.");
                else{

                    try{
                        TacheUnitaire& t = (ProjetManager::getInstance().trouverProjet(ui->comboBoxProjet->currentText())->ajouterTacheUnitaire(id,titre, disp,ech, d, pre));
                         //ajouter précédence
                        try {
                            for(QMap<QString, Tache*>::const_iterator it= predecesseurs.cbegin();it!=predecesseurs.cend();it++){
                                t.ajouterPrecedence(*(*it));
                            }
                            //ajouter tache mère
                            if(ui->comboBoxMere->currentText()!=QString(" ")) {
                                static_cast<TacheComposite*>(ProjetManager::getInstance().trouverTache(ui->comboBoxMere->currentText()))->ajouterSousTache(t);
                            }
                            //emit nouvelleTache();
                            MessageValidation mv(QString("Succès"), QString("La tache "+ui->lineEditId->text()+" a été ajoutée."));
                            int rep = mv.exec();
                            if (rep == QMessageBox::No)
                                close();
                            else { ui->lineEditId->clear(); ui->lineEditTitre->clear(); remplirComboMere(ui->comboBoxProjet->currentText()); }
                        }
                        catch(CalendarException e){
                            QMessageBox::warning(this, "Erreur", e.getInfo());
                            ProjetManager::getInstance().trouverProjet(ui->comboBoxProjet->currentText())->removeTache(&t);
                        }
                    }
                    catch(CalendarException e){
                        QMessageBox::warning(this, "Erreur", e.getInfo());
                    }

                }
            }
            else if(ui->radioButtonComp->isChecked()){
                try{
                    TacheComposite& t = (ProjetManager::getInstance().trouverProjet(ui->comboBoxProjet->currentText())->ajouterTacheComposite(id, titre, disp, ech));
                    //ajouter précédence
                    try {
                        for(QMap<QString, Tache*>::const_iterator it= predecesseurs.cbegin();it!=predecesseurs.cend();it++){
                            t.ajouterPrecedence(*(*it));
                        }
                        //ajouter tache mère
                        if(ui->comboBoxMere->currentText()!=QString(" ")) {
                             static_cast<TacheComposite*>(ProjetManager::getInstance().trouverTache(ui->comboBoxMere->currentText()))->ajouterSousTache(t);
                        }
                        MessageValidation mv(QString("Succès"), QString("La tache "+ui->lineEditId->text()+" a été ajoutée."));
                        int rep = mv.exec();
                        if (rep == QMessageBox::No)
                            close();
                        else { ui->lineEditId->clear(); ui->lineEditTitre->clear(); remplirComboMere(ui->comboBoxProjet->currentText()); }
                    }
                    catch(CalendarException e){
                        QMessageBox::warning(this, "Erreur", e.getInfo());
                        ProjetManager::getInstance().trouverProjet(ui->comboBoxProjet->currentText())->removeTache(&t);

                    }
                    //ajouter sous taches*/
                }
                catch(CalendarException e){QMessageBox::warning(this, "Erreur", e.getInfo());}
            }
            else
                QMessageBox::warning(this, "Attention", "Veuillez renseigner le champ 'type de tache'");
        }
    }
}

EditeurTache::~EditeurTache() { delete ui; }

/*///////////////////////////////////////////////////////*/
/*                     AjoutPrecedence                   */
/*///////////////////////////////////////////////////////*/


AjoutPrecedence::AjoutPrecedence(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::AjoutPrecedence), nbTachesAjoutees(0)
{
    ui->setupUi(this);
    this->setModal(true);
    ui->treeView->setEditTriggers(0);
    EditeurTache::predecesseurs.clear();

    for(QMap<QString, Projet*>::const_iterator it= ProjetManager::getInstance().getProjets().cbegin();
        it!=ProjetManager::getInstance().getProjets().cend();
        it++){//parcours des projets

           QStandardItem* item = new QStandardItem((*it)->getNom());
           QFont f = QFont();
           f.setBold(true);
           f.setUnderline(true);
           item->setFont(f);
           modele.appendRow(item);

           for(QMap<QString, Tache*>::const_iterator it2 = (*it)->getTaches().cbegin();
                it2!=(*it)->getTaches().cend();
                it2++){//parcours des taches de chaque projet

                    QStandardItem* item2 = new QStandardItem((*it2)->getId());//création de l'identifiant de la tache
                    item2->setCheckable(true); //ajout de la checkbox
                    item->appendRow(item2); //ajout de item2 au projet
                    nbTachesAjoutees++;
            }
    }
    ui->treeView->setModel(&modele);

    QObject::connect(ui->pushButtonAnnuler, SIGNAL(clicked()),this,SLOT(close()));
    QObject::connect(ui->pushButtonOK, SIGNAL(clicked()),this,SLOT(EnregistrerPred()));
}

void AjoutPrecedence::EnregistrerPred(){
    //attribut modele à la classe ajoutPrecedence
    QMap<QString, Tache*> pred;
    for(int nb=0; nb<modele.rowCount(); nb++){
        QList<QString>* liste = new QList<QString>;
        testChecked(modele.item(nb), liste);

        for(QList<QString>::const_iterator i = liste->cbegin(); i< liste->cend(); i++){
            Projet* p =ProjetManager::getInstance().trouverProjet(modele.item(nb)->text());
            Tache* t= p->trouverTache(*i);
            pred.insert(t->getId(), t);
        }
    }
    EditeurTache::predecesseurs=pred;
    close();
}

QList<QString>* AjoutPrecedence::testChecked(QStandardItem* element, QList<QString>* liste){
    QStandardItem* temp;
    for(int nb=0; nb<element->rowCount(); nb++){
        temp=element->child(nb);
        if(temp->checkState()==Qt::Checked){
             liste->append(temp->text());
        }
        else{
            if (temp->child(0)!=0){
                testChecked(temp, liste);
            }
        }
    }
    return liste;
}

AjoutPrecedence::~AjoutPrecedence() { delete ui; }

/*///////////////////////////////////////////////////////*/
/*                     Nouvelle Précédence               */
/*///////////////////////////////////////////////////////*/

NouvellePrecedence::NouvellePrecedence(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::NouvellePrecedence)
{
    ui->setupUi(this);

    //boucle pour remplir la combobox proposant les projets
    ui->comboBoxProjet1->addItem("Projets");
    for(QMap<QString, Projet*>::const_iterator it= ProjetManager::getInstance().getProjets().cbegin();it!=ProjetManager::getInstance().getProjets().cend(); it++){
        ui->comboBoxProjet1->addItem((*it)->getNom());
    }

    ui->comboBoxProjet2->addItem("Projets");
    for(QMap<QString, Projet*>::const_iterator it= ProjetManager::getInstance().getProjets().cbegin();it!=ProjetManager::getInstance().getProjets().cend(); it++){
        ui->comboBoxProjet2->addItem((*it)->getNom());
    }

     QObject::connect(ui->comboBoxProjet1, SIGNAL(currentIndexChanged(QString)), this, SLOT(remplirComboTache1(QString))); //affiche les tâches possibles en fonction du projet séléctionné
     QObject::connect(ui->comboBoxProjet2, SIGNAL(currentIndexChanged(QString)), this, SLOT(remplirComboTache2(QString))); //affiche les tâches possibles en fonction du projet séléctionné
     QObject::connect(ui->pushButtonAnnuler, SIGNAL(clicked()), this, SLOT(close()));
     QObject::connect(ui->pushButtonValider, SIGNAL(clicked()), this, SLOT(EnregistrerPrec()));
}

void NouvellePrecedence::remplirComboTache1(QString p){
    ui->comboBoxTache1->clear();
    if(ui->comboBoxProjet1->currentText()!="Projets"){
        Projet* pr =ProjetManager::getInstance().trouverProjet(p);
        for(QMap<QString, Tache*>::const_iterator it=pr->getTaches().cbegin(); it!=pr->getTaches().cend(); it++){
             ui->comboBoxTache1->addItem((*it)->getId());
        }
    }
}

void NouvellePrecedence::remplirComboTache2(QString p){
    ui->comboBoxTache2->clear();
    if(ui->comboBoxProjet2->currentText()!="Projets"){
        Projet* pr =ProjetManager::getInstance().trouverProjet(p);
        for(QMap<QString, Tache*>::const_iterator it=pr->getTaches().cbegin(); it!=pr->getTaches().cend(); it++){
             ui->comboBoxTache2->addItem(it.value()->getId());
        }
    }
}

void NouvellePrecedence::EnregistrerPrec(){
    Tache* t1 =ProjetManager::getInstance().trouverProjet(ui->comboBoxProjet1->currentText())->trouverTache(ui->comboBoxTache1->currentText());
    Tache* t2 = ProjetManager::getInstance().trouverProjet(ui->comboBoxProjet2->currentText())->trouverTache(ui->comboBoxTache2->currentText());
    try{
    t1->ajouterPrecedence(*t2);
    }catch(CalendarException e){QMessageBox::warning(this, "Erreur", e.getInfo()); return;}
    close();
}

NouvellePrecedence::~NouvellePrecedence() { delete ui; }

/*///////////////////////////////////////////////////////*/
/*                     Ajout Sous-Tâche                  */
/*///////////////////////////////////////////////////////*/

AjoutSousTache::AjoutSousTache(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::AjoutSousTache)
{
    ui->setupUi(this);

    //boucle pour remplir la combobox proposant les projets
    ui->comboBoxProjet->addItem(" ");
    for(QMap<QString, Projet*>::const_iterator it= ProjetManager::getInstance().getProjets().cbegin();it!=ProjetManager::getInstance().getProjets().cend(); it++){
        if (!(*it)->getTachesComposites().empty())
            ui->comboBoxProjet->addItem((*it)->getNom());
    }

     QObject::connect(ui->comboBoxProjet, SIGNAL(currentIndexChanged(QString)), this, SLOT(remplirComboTacheMere(QString))); //affiche les tâches possibles en fonction du projet séléctionné
     QObject::connect(ui->comboBoxProjet, SIGNAL(currentIndexChanged(QString)), this, SLOT(remplirComboSousTache(QString))); //affiche les tâches possibles en fonction du projet séléctionné
     QObject::connect(ui->pushButtonValider, SIGNAL(clicked()), this, SLOT(EnregistrerSousTache()));
     QObject::connect(ui->pushButtonValider, SIGNAL(clicked()),parent,SLOT(updateTree()));
     QObject::connect(ui->pushButtonAnnuler, SIGNAL(clicked()), this, SLOT(close()));
     QObject::connect(this, SIGNAL(finished(int)),parent,SLOT(updateTree()));
}

void AjoutSousTache::remplirComboTacheMere(const QString& p){
    ui->comboBoxTacheMere->clear();
    ui->comboBoxTacheMere->addItem(" ");
    if(p!=" "){
        Projet* projet =ProjetManager::getInstance().trouverProjet(p);
        QMap<QString, Tache*> tachesComposites= projet->getTachesComposites();
        for(QMap<QString, Tache*>::const_iterator it=tachesComposites.begin(); it!=tachesComposites.end(); ++it){
             ui->comboBoxTacheMere->addItem((*it)->getId());
        }
    }
}

void AjoutSousTache::remplirComboSousTache(const QString& p){
    ui->comboBoxSousTache->clear();
    ui->comboBoxSousTache->addItem(" ");
    if(p!=" "){
        Projet* pr =ProjetManager::getInstance().trouverProjet(p);
        for(QMap<QString, Tache*>::const_iterator it=pr->getTaches().begin(); it!=pr->getTaches().end(); ++it){
            if((*it)->getTacheMere()==0)
                ui->comboBoxSousTache->addItem((*it)->getId());
        }
    }
}

void AjoutSousTache::EnregistrerSousTache(){
    try{
        if (ui->comboBoxSousTache->currentText()!=QString(" ") && ui->comboBoxTacheMere->currentText()!=QString(" ")) {
            TacheComposite* mere =static_cast<TacheComposite*>(ProjetManager::getInstance().trouverTache(ui->comboBoxTacheMere->currentText()));
            Tache* soustache = ProjetManager::getInstance().trouverTache(ui->comboBoxSousTache->currentText());
            mere->ajouterSousTache(*soustache);
            MessageValidation mv(QString("Succès"), QString("La sous-tâche "+ui->comboBoxSousTache->currentText()+" a été ajoutée à "+ui->comboBoxTacheMere->currentText()+"."));
            int rep = mv.exec();
            if (rep == QMessageBox::No)
                close();
            else { remplirComboSousTache(ui->comboBoxProjet->currentText()); remplirComboTacheMere(ui->comboBoxProjet->currentText()); }
        }
        else
            throw CalendarException("Veuillez séléctionner des tâches");
    }
    catch(CalendarException e){
        QMessageBox::warning(this, "Erreur Ajout", e.getInfo());
    }
}

AjoutSousTache::~AjoutSousTache() { delete ui; }

/*///////////////////////////////////////////////////////*/
/*                   Suppression Tâche                   */
/*///////////////////////////////////////////////////////*/

SuppressionTache::SuppressionTache(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::SuppressionTache)
{
    ui->setupUi(this);

    //boucle pour remplir la combobox proposant les projets
    ui->comboBoxProjet->addItem(" ");
    for(QMap<QString, Projet*>::const_iterator it= ProjetManager::getInstance().getProjets().cbegin();it!=ProjetManager::getInstance().getProjets().cend(); it++){
        if (!(*it)->getTaches().empty())
            ui->comboBoxProjet->addItem((*it)->getNom());
    }

     QObject::connect(ui->comboBoxProjet, SIGNAL(currentIndexChanged(QString)), this, SLOT(remplirComboTache(QString))); //affiche les tâches possibles en fonction du projet séléctionné
     QObject::connect(ui->pushButtonValider, SIGNAL(clicked()), this, SLOT(supprimerTache()));
     QObject::connect(ui->pushButtonValider, SIGNAL(clicked()),parent,SLOT(updateTree()));
     QObject::connect(ui->pushButtonAnnuler, SIGNAL(clicked()), this, SLOT(close()));
     QObject::connect(this, SIGNAL(finished(int)),parent,SLOT(updateTree()));
}

void SuppressionTache::remplirComboTache(const QString& p){
    ui->comboBoxTache->clear();
    Projet* projet =ProjetManager::getInstance().trouverProjet(p);
    if(projet!=0){
        ui->comboBoxTache->addItem(" ");
        QMap<QString, Tache*> taches= projet->getTaches();
        for(QMap<QString, Tache*>::const_iterator it=taches.begin(); it!=taches.end(); ++it){
             ui->comboBoxTache->addItem((*it)->getId());
             qDebug()<<(*it)->getId();
        }
    }
}

void SuppressionTache::supprimerTache(){
    try{
        if (ui->comboBoxTache->currentText()!=QString(" ")) {
            Projet* projet=ProjetManager::getInstance().trouverProjet(ui->comboBoxProjet->currentText());
            Tache* tache = projet->trouverTache(ui->comboBoxTache->currentText());
            projet->removeTache(tache);
            MessageValidation mv(QString("Succès"), QString("La tâche "+ui->comboBoxTache->currentText()+" a été supprimée."));
            int rep = mv.exec();
            if (rep == QMessageBox::No)
                close();
            else { remplirComboTache(ui->comboBoxProjet->currentText()); }
        }
        else
            throw CalendarException("Veuillez séléctionner une tâche");
    }
    catch(CalendarException e){
        QMessageBox::warning(this, "Erreur Suppression", e.getInfo());
    }
}

SuppressionTache::~SuppressionTache() { delete ui; }

/*///////////////////////////////////////////////////////*/
/*                 Supprimmer Précédence                 */
/*///////////////////////////////////////////////////////*/

SuppressionPrecedence::SuppressionPrecedence(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::SuppressionPrecedence)
{
    ui->setupUi(this);

    //boucle pour remplir la combobox proposant les projets
    ui->comboBoxProjet->addItem(" ");
    for(QMap<QString, Projet*>::const_iterator it= ProjetManager::getInstance().getProjets().cbegin();it!=ProjetManager::getInstance().getProjets().cend(); it++){
        ui->comboBoxProjet->addItem((*it)->getNom());
    }

     QObject::connect(ui->comboBoxProjet, SIGNAL(currentIndexChanged(QString)), this, SLOT(remplirComboTache(QString))); //affiche les tâches possibles en fonction du projet séléctionné
     QObject::connect(ui->comboBoxTache, SIGNAL(currentIndexChanged(QString)), this, SLOT(showPrecedence(QString))); //affiche les tâches possibles en fonction du projet séléctionné
     QObject::connect(ui->pushButtonAnnuler, SIGNAL(clicked()), this, SLOT(close()));
     QObject::connect(ui->pushButtonValider, SIGNAL(clicked()), this, SLOT(supprimerPrec()));
}

void SuppressionPrecedence::remplirComboTache(const QString& p){
    ui->comboBoxTache->clear();
    Projet* projet =ProjetManager::getInstance().trouverProjet(p);
    if(projet!=0){
        ui->comboBoxTache->addItem(" ");
        QMap<QString, Tache*> taches= projet->getTaches();
        for(QMap<QString, Tache*>::const_iterator it=taches.begin(); it!=taches.end(); ++it){
             ui->comboBoxTache->addItem((*it)->getId());
             qDebug()<<(*it)->getId();
        }
    }
}

void SuppressionPrecedence::showPrecedence(const QString& t){
    ui->listPrecedence->clear();
    try {
        Tache* tache=ProjetManager::getInstance().trouverTache(t);
        if(tache!=0){
            Tache* tache=ProjetManager::getInstance().trouverTache(t);
            for(QMap<QString, Tache*>::const_iterator it=tache->getPred().begin(); it!=tache->getPred().end(); it++){
                 ui->listPrecedence->addItem((*it)->getId());
            }
        }
    }
    catch(CalendarException e){
        QMessageBox::warning(this, "Erreur Suppression", e.getInfo());
    }
}

void SuppressionPrecedence::supprimerPrec() {
    try {
        QString t=ui->comboBoxTache->currentText();
        QString p=ui->listPrecedence->currentItem()->text();
        Tache* tache=ProjetManager::getInstance().trouverTache(t);
        Tache* pred=ProjetManager::getInstance().trouverTache(p);
        if (tache!=0 && pred!=0) {
            tache->removePrecedence(pred);
            pred->removeSuivant(tache);
            MessageValidation mv(QString("Succès"), QString(p+" ne précède plus "+t+"."));
            int rep = mv.exec();
            if (rep == QMessageBox::No)
                close();
            else { showPrecedence(t); }
        }
        else
            throw CalendarException("Veuillez sélectionner des tâches.");
    }
    catch(CalendarException e){QMessageBox::warning(this, "Erreur", e.getInfo());}
}

SuppressionPrecedence::~SuppressionPrecedence() { delete ui; }

/*///////////////////////////////////////////////////////*/
/*                 Supprimer Sous-Tâche                  */
/*///////////////////////////////////////////////////////*/

SupprimerSousTache::SupprimerSousTache(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::SupprimerSousTache)
{
    ui->setupUi(this);

    //boucle pour remplir la combobox proposant les projets
    ui->comboBoxProjet->addItem(" ");
    for(QMap<QString, Projet*>::const_iterator it= ProjetManager::getInstance().getProjets().cbegin();it!=ProjetManager::getInstance().getProjets().cend(); it++){
        if (!(*it)->getTachesComposites().empty())
            ui->comboBoxProjet->addItem((*it)->getNom());
    }

     QObject::connect(ui->comboBoxProjet, SIGNAL(currentIndexChanged(QString)), this, SLOT(remplirComboTacheMere(QString))); //affiche les tâches possibles en fonction du projet séléctionné
     QObject::connect(ui->comboBoxTacheMere, SIGNAL(currentIndexChanged(QString)), this, SLOT(remplirComboSousTache(QString))); //affiche les tâches possibles en fonction du projet séléctionné
     QObject::connect(ui->pushButtonValider, SIGNAL(clicked()), this, SLOT(suppressionSousTache()));
     QObject::connect(ui->pushButtonValider, SIGNAL(clicked()),parent,SLOT(updateTree()));
     QObject::connect(ui->pushButtonAnnuler, SIGNAL(clicked()), this, SLOT(close()));
     QObject::connect(this, SIGNAL(finished(int)),parent,SLOT(updateTree()));
}

void SupprimerSousTache::remplirComboTacheMere(const QString& p){
    ui->comboBoxTacheMere->clear();
    ui->comboBoxTacheMere->addItem(" ");
    if(p!=" "){
        Projet* projet =ProjetManager::getInstance().trouverProjet(p);
        QMap<QString, Tache*> tachesComposites= projet->getTachesComposites();
        for(QMap<QString, Tache*>::const_iterator it=tachesComposites.begin(); it!=tachesComposites.end(); ++it){
            if (!(static_cast<TacheComposite*>(*it)->getSousTaches().empty()))
                ui->comboBoxTacheMere->addItem((*it)->getId());
        }
    }
}

void SupprimerSousTache::remplirComboSousTache(const QString& t){
    ui->comboBoxSousTache->clear();
    ui->comboBoxSousTache->addItem(" ");
    if(t!=" "){
        TacheComposite* mere = static_cast<TacheComposite*>(ProjetManager::getInstance().trouverTache(t));
        if (mere!=0) {
            for(QMap<QString, Tache*>::const_iterator it=mere->getSousTaches().begin(); it!=mere->getSousTaches().end(); ++it){
                ui->comboBoxSousTache->addItem((*it)->getId());
            }
        }
    }
}

void SupprimerSousTache::suppressionSousTache(){
    try{
        if (ui->comboBoxSousTache->currentText()!=QString(" ") && ui->comboBoxTacheMere->currentText()!=QString(" ")) {
            TacheComposite* mere =static_cast<TacheComposite*>(ProjetManager::getInstance().trouverTache(ui->comboBoxTacheMere->currentText()));
            Tache* soustache = ProjetManager::getInstance().trouverTache(ui->comboBoxSousTache->currentText());
            mere->removeSousTache(soustache);
            MessageValidation mv(QString("Succès"), QString("La sous-tâche "+ui->comboBoxSousTache->currentText()+" a été enlevée à "+ui->comboBoxTacheMere->currentText()+"."));
            int rep = mv.exec();
            if (rep == QMessageBox::No)
                close();
            else { remplirComboTacheMere(ui->comboBoxProjet->currentText()); }
        }
        else
            throw CalendarException("Veuillez séléctionner des tâches");
    }
    catch(CalendarException e){
        QMessageBox::warning(this, "Erreur Suppression", e.getInfo());
    }
}

SupprimerSousTache::~SupprimerSousTache() { delete ui; }

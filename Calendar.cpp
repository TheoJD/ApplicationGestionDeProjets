#include "Calendar.h"

/**
 * \file Calendar.cpp
 * \brief Defini les methodes de Calendar.h
 */

/**************************************/
/*               DUREE                */
/**************************************/

QTextStream& operator<<(QTextStream& f, const Duree & d){ d.afficher(f); return f; }

QTextStream& operator>>(QTextStream& flot, Duree& duree){
    unsigned int h,m;
    bool ok=true;
    flot>>h;
    if (flot.status()!=QTextStream::Ok) ok=false;

    if(flot.read(1)=="H") {
        flot>>m;
        if (flot.status()!=QTextStream::Ok) ok=false;
    }
    else {
        ok=false;
    }
    if (ok) duree=Duree(h,m);
    return flot;
}

QString Duree::stringDuree() const {
    if (getMinute()<10)
        return QString::number(getHeure())+"h"+QString::number(0)+(QString::number(getMinute()));
    else
        return QString::number(getHeure())+"h"+(QString::number(getMinute()));
}

void Duree::afficher(QTextStream& f) const {
    f.setPadChar('0');
    f.setFieldWidth(2);
    f<<nb_minutes/60;
    f.setFieldWidth(0);
    f<<"H";
    f.setFieldWidth(2);
    f<<nb_minutes%60;
    f.setFieldWidth(0);
    f.setPadChar(' ');
}

Duree operator+(const Duree& d1, const Duree& d2) {
    Duree d(0);
    d.setDuree(d1.getDureeEnMinutes()+d2.getDureeEnMinutes());
    return d;
}

Duree operator-(const Duree& d1, const Duree& d2) {
    Duree d(0);
    d.setDuree(d1.getDureeEnMinutes()-d2.getDureeEnMinutes());
    return d;
}

Duree operator+=(Duree& d1, const Duree& d2) { return d1=d1+d2; }

bool operator<=(const Duree& d1, const Duree& d2) { return d1.getDureeEnMinutes()<=d2.getDureeEnMinutes(); }
bool operator>=(const Duree& d1, const Duree& d2) { return d1.getDureeEnMinutes()>=d2.getDureeEnMinutes(); }
bool operator<(const Duree& d1, const Duree& d2) { return d1.getDureeEnMinutes()<d2.getDureeEnMinutes(); }
bool operator>(const Duree& d1, const Duree& d2) { return d1.getDureeEnMinutes()>d2.getDureeEnMinutes(); }
bool operator==(const Duree& d1, const Duree& d2) { return d1.getDureeEnMinutes()==d2.getDureeEnMinutes(); }
bool operator!=(const Duree& d1, const Duree& d2) { return !(d1==d2); }

QDateTime operator+(const QDateTime& dt, const Duree& dur) {
    return dt.addSecs(dur.getDureeEnSecondes());
}

/************************************************************************************************************************************************/

    /**************************************/
    /*               TACHE                */
    /**************************************/

QTextStream& operator<<(QTextStream& fout, const Tache& t){
    fout<<t.getId()<<"\n";
    fout<<t.getTitre()<<"\n";
    fout<<t.getDateDisponibilite().toString()<<"\n";
    fout<<t.getDateEcheance().toString()<<"\n";
    return fout;
}

Tache::~Tache() {
}

void Tache::setId(const QString& str){
  if (ProjetManager::getInstance().isTacheExistante((str))) throw CalendarException("erreur Tache : tache "+str+" déjà existante");
  identificateur=str;
}

void Tache::setTacheMere(const TacheComposite* mere) {
    if (mere!=tacheMere) {
        if (mere->getProjet()!=projet)
            throw CalendarException("erreur Tache : la tache mere et la sous-tache doivent appartenir au même projet");
        tacheMere=mere;
    }
}

void Tache::removeTacheMere(){
    tacheMere=0;
}

bool Tache::hasAlreadyPrecedence(const Tache* t) const {
    return t->isPrecedentDe(this);
}

bool Tache::isPrecedentDe(const Tache* t) const {
    if (t->getPred().empty()) {
        if (t->getTacheMere()) {
            return isPrecedentDe(t->getTacheMere()); //Sera true si *this précède la tache Mère de t.
        }
        else return false; //Si la tache t n'a pas de precedence, *this n'est pas precedent de t.
    }
    else {
        QMap<QString, Tache*>::const_iterator it = t->getPred().find(identificateur);
        if (it!=t->getPred().end()) return true; //Si *this est dans les taches directement precedentes de t, alors on retourne true.

        bool resultat=false;
        it = t->getPred().begin();
        while (it!=t->getPred().end() && !resultat) {
            resultat=isPrecedentDe(*it); //Sera à true si une tache precedente de t est precedee par *this, *this precede t.
            if (!resultat && (*it)->isComposite())
                resultat=isSousTache(static_cast<const TacheComposite*>(*it)); //Sera true si *this est sous-tache d'une tache qui precede t.
            it++;
        }
        if (!resultat && t->getTacheMere()) {
            resultat=isPrecedentDe(t->getTacheMere()); //Sera true si *this précède la tache Mère de t.
        }
        return resultat;
    }
}

void Tache::verifAjoutPrecedence(const Tache &t) const {
    if (&t==this)
        throw CalendarException("Une tache ne peut pas s'auto-preceder");
    if(t.getDateDisponibilite()>echeance)
        throw CalendarException(t.getId()+" n'est disponible qu'apres "+getId()+", impossible d'impliquer une relation de precedence");
    if(isPrecedentDe(&t))
        throw CalendarException("On ne peut pas impliquer des precedences dans les deux sens !");
    if (hasAlreadyPrecedence(&t))
        throw CalendarException("La precedence existe deja, de maniere directe ou indirecte.");
    if (t.isComposite()) {
        if ((*this).isSousTache(static_cast<const TacheComposite*>(&t)))
            throw CalendarException("Erreur Tache : "+getId()+" est deja sous-tache de "+t.getId()+", il ne peut y avoir de precedence.");
        QMap<QString,Tache*>::const_iterator it=static_cast<const TacheComposite*>(&t)->getSousTaches().begin();
        while (it!=static_cast<const TacheComposite*>(&t)->getSousTaches().end()) {
            if (this->isPrecedentDe(*it))
                throw CalendarException("Erreur Tache : une sous-tache de "+t.getId()+" est deja precedente de "+getId()+", on ne peut impliquer de precedence dans les deux sens");
            it++;
        }
    }
}

void Tache::ajouterPrecedence(Tache &t) {
    verifAjoutPrecedence(t);
    pred.insert(t.getId(), &t);
    t.ajouterSuivant(this);
}

void Tache::ajouterSuivant(Tache* t) {
    suiv.insert(t->getId(), t);
}

void Tache::afficherPrecedences() const {
    if (pred.empty()) {
        qDebug()<<"Aucune precedence pour "<<getId().toStdString().c_str();
    }
    else {
        qDebug()<<"Precedences de "<< getId().toStdString().c_str() <<" :";
        for(QMap<QString, Tache*>::const_iterator it=pred.begin(); it!=pred.end(); it++)
            qDebug()<<(*it)->getId().toStdString().c_str();
    }
}

bool Tache::hasSuivant(const Tache* t) const {
    return isPrecedentDe(t);
}

bool Tache::isSuivantDe(const Tache* t) const {
    return t->isPrecedentDe(this);
}

void Tache::afficherSuivant() const {
    if (suiv.empty()) {
        qDebug()<<"Aucune precedence pour "<<getId().toStdString().c_str();
    }
    else {
        qDebug()<<"Suivants de "<< getId().toStdString().c_str() <<" :";
        for(QMap<QString, Tache*>::const_iterator it=suiv.begin(); it!=suiv.end(); it++)
            qDebug()<<(*it)->getId().toStdString().c_str();
    }
}

bool Tache::isSousTache(const TacheComposite *t) const {
    if (tacheMere==0) return false;
    if (tacheMere==t) return true;
    else return tacheMere->isSousTache(t);
}

bool Tache::arePrecedencesProgrammeesAvant(const QDateTime& dt) const{
    if (pred.size()==0) return true;
    QMap<QString, Tache*>::const_iterator it=pred.begin();
    bool resultat=true;
    if (tacheMere!=0)
        resultat=tacheMere->arePrecedencesProgrammeesAvant(dt);
    while (resultat && it!=pred.end()){
        resultat=(*it)->isProgrameeAvant(dt);
        it++;
    }
    return resultat;
}

void Tache::removeTacheDePred() {
    QMap<QString,Tache*>::iterator it=suiv.begin();
    while (it!=suiv.end()) {
        (*it)->removePrecedence(this);
        it=suiv.erase(it);
    }
}

void Tache::removeTacheDeSuiv() {
    QMap<QString,Tache*>::iterator it=pred.begin();
    while (it!=pred.end()) {
        (*it)->removeSuivant(this);
        it=pred.erase(it);
    }
}

/************************************************************************************************************************************************/

    /**************************************/
    /*           TACHE UNITAIRE           */
    /**************************************/

QTextStream& operator<<(QTextStream& fout, const TacheUnitaire& t){
    fout << static_cast<const Tache &>(t);
    fout<<t.getDuree()<<"\n";
    return fout;
}

TacheUnitaire::~TacheUnitaire() {}

void TacheUnitaire::verifAjoutPrecedence(const Tache &t) const {
    Tache::verifAjoutPrecedence(t);
    if (dureeRestanteAProgrammer()<duree && !(t.isProgramee()))
        throw CalendarException("La tache "+getId()+" est deja au moins en partie programmae mais pas "+t.getId()+", donc il n'est pas possible d'ajouter une relation de precedence");
}

bool TacheUnitaire::isProgramee() const {
    ProgrammationManager& pgm=ProgrammationManager::getInstance();
    Duree resultat=Duree(0);
    QMap<QDateTime, Programmation*>::const_iterator it = pgm.getProgrammations().begin();
    while (it!= pgm.getProgrammations().end() && (*it)->getDate()<getDateDisponibilite()){  ++it; }
    while (it!= pgm.getProgrammations().end() && (*it)->getDate()<=getDateEcheance()) { // <! Si la programmation se trouve dans les dates de dispo de la tache...
        if ((*it)->getTache()==this) // <! ... et qu'elle concerne la tache...
            resultat += (*it)->getDuree(); // <! ... on ajoute la durée de la programmation.
        ++it;
    }
    return (resultat==duree);
}

bool TacheUnitaire::isProgrameeAvant(const QDateTime& dt) const {
    ProgrammationManager& pgm=ProgrammationManager::getInstance();
    bool dateTimeCoherente=true; /*<! Cela va nous permettre de vérifier que chaque fin de programamtion de la tache
                                    est antérieure à la date et l'horaire renseignées en arguments. */
    QDateTime dtFinProg;
    Duree resultat=Duree(0);
    QMap<QDateTime, Programmation*>::const_iterator it=pgm.getProgrammations().begin();
    while (it!= pgm.getProgrammations().end() && (*it)->getDate()<getDateDisponibilite()) { ++it; } // <! On avance jusqu'aux programmations pouvant concerner la tache
    while (it!= pgm.getProgrammations().end() && (*it)->getDate()<=getDateEcheance() && resultat<duree && dateTimeCoherente){
                                                                                 /*<! Si la programmation se trouve dans les dates de dispo de la tache,
                                                                                    qu'on a pas atteint la durée totale de la tache,
                                                                                    et que les dates des programamtions de la atche sont cohérentes...*/
        if ((*it)->getTache()==this) {
            dtFinProg=((*it)->getDateTime()+(*it)->getDuree()); // <! On définit la date à laquelle la tache est finie, qui l'ajout de la duree de la programmation à la date de programamtion.
            dateTimeCoherente=(dt>=dtFinProg); // <! On vérifie que dt est bien postérieure à la date de fin de programmation
            if (dateTimeCoherente) {
                resultat += (*it)->getDuree(); // <! Si tous les critères sont remplis, on ajoute la durée de la programmation.
            }
        }
        ++it;
    }
    return (resultat==duree);
}

Duree TacheUnitaire::dureeRestanteAProgrammer() const {
    ProgrammationManager& pgm=ProgrammationManager::getInstance();
    Duree resultat=Duree(0);
    QMap<QDateTime, Programmation*>::const_iterator it=pgm.getProgrammations().begin();
    while (it!= pgm.getProgrammations().end() && ((*it)->getDateTime().date())<getDateDisponibilite()) { ++it; } // <! On avance jusqu'aux programmations pouvant concerner la tache
    while (it!= pgm.getProgrammations().end() && (*it)->getDate()<=getDateEcheance()){ // <! Si la programmation se trouve dans les dates de dispo de la tache,
        if ((*it)->getTache()==this) { // Et qu'elle concerne  la atche
            resultat += (*it)->getDuree(); // <! On ajoute la durée de la programmation.
        }
        ++it; // <! La duree restante est la difference entre la duree de la tache unitaire et la duree totale déjà programmée de cette tache.
    }
    return (duree-resultat); // <! La duree restante est la difference entre la duree de la tache unitaire et la duree totale déjà programmée de cette tache.
}

/************************************************************************************************************************************************/

    /**************************************/
    /*           TACHE COMPOSITE          */
    /**************************************/

QTextStream& operator<<(QTextStream& fout, const TacheComposite& t){
    fout << static_cast<const Tache &>(t);
    fout<<t.getDuree()<<"\n";
    return fout;
}

TacheComposite::~TacheComposite() {
}

void TacheComposite::setTacheMere(const TacheComposite* mere) {
    if (mere!=getTacheMere()) {
        if (hasAlreadySousTache(mere))
            throw CalendarException("erreur Tache : "+mere->getId()+" est deja sous-tache de "+getId()+", on ne peut impliquer une relation dans les deux sens");
        Tache::setTacheMere(mere);
    }
}

Duree TacheComposite::getDuree() const {
    if (sousTaches.empty()) {
        throw CalendarException("La Tache n'a aucune sous-tache...");
    }
    else {
        Duree d(0);
        for(QMap<QString, Tache*>::const_iterator it=sousTaches.begin(); it!=sousTaches.end(); it++){
            Duree dtemp = (*it)->getDuree();
            d=d+dtemp;
        }
        return d;
    }
}

bool TacheComposite::hasAlreadySousTache(const Tache* t) const
{
    return (t->isSousTache(this));
}

void TacheComposite::ajouterSousTache(Tache& t) {
    if (&t==this)
        throw CalendarException("Les deux taches sont identiques");
    if (t.getProjet()!=getProjet())
        throw CalendarException("Les deux taches doivent provenir du meme projet");
    if (t.getDateDisponibilite()<getDateDisponibilite() || t.getDateEcheance()>getDateEcheance())
        throw CalendarException("Erreur TacheComposiste : les taches ont des dates en conflit, impossible d'ajouter la sous-tache.");
    if(t.isComposite() && isSousTache(static_cast<TacheComposite*>(&t)))
        throw CalendarException("On ne peut pas impliquer des sous-taches dans les deux sens !");
    if (hasAlreadySousTache(&t))
        throw CalendarException(t.getId()+" est deja sous-tache (directement ou indirectement) de "+getId()+".");
    if(isPrecedentDe(&t))
        throw CalendarException(t.getId()+" est deja liee par une relation de precedence à "+t.getId()+", donc on ne peut impliquer une relation de sous-tache.");
    if (hasAlreadyPrecedence(&t))
        throw CalendarException(t.getId()+" a deja "+t.getId()+" comme precedence, donc on ne peut impliquer une relation de sous-tache.");

    sousTaches.insert(t.getId(), &t);
    t.setTacheMere(this);
}

void TacheComposite::afficherSousTaches() const {
    if (sousTaches.empty()) {
        qDebug()<<"Aucune sous-tache pour "<<getId().toStdString().c_str();
    }
    else {
        qDebug()<<"Sous-tache(s) de "<< getId().toStdString().c_str() <<" :";
        for(QMap<QString, Tache*>::const_iterator it=sousTaches.begin(); it!=sousTaches.end(); it++)
            qDebug()<<(*it)->getId().toStdString().c_str();
    }
}

void TacheComposite::removeTacheDeMere() {
    for(QMap<QString,Tache*>::iterator it=sousTaches.begin(); it!=sousTaches.end(); ++it)
        (*it)->removeTacheMere();
}

void TacheComposite::verifAjoutPrecedence(const Tache &t) const {
    Tache::verifAjoutPrecedence(t);
    if (t.isComposite() && (*this).isSousTache(static_cast<const TacheComposite*>(&t)))
        throw CalendarException("Erreur Tache : "+getId()+" est deja sous-tache de "+t.getId()+", il ne peut y avoir de precedence.");
}

bool TacheComposite::isProgramee() const {
    if (sousTaches.empty()) return true;
    QMap<QString, Tache*>::const_iterator it=sousTaches.begin();
    bool resultat=true;
    while (resultat && it!=sousTaches.end()){
        resultat=(*it)->isProgramee();
        it++;
    }
    return resultat;
}

bool TacheComposite::isProgrameeAvant(const QDateTime& dt) const {
    if (sousTaches.empty()) return true;
    QMap<QString, Tache*>::const_iterator it=sousTaches.begin();
    bool resultat=true;
    while (resultat && it!=sousTaches.end()){
        resultat=(*it)->isProgrameeAvant(dt);
        it++;
    }
    return resultat;
}

/************************************************************************************************************************************************/

    /**************************************/
    /*               PROJET               */
    /**************************************/

Projet::Projet(const QString& n, const QDate& dispo, const QDate& deadline) {
    if (dispo>deadline)
        throw CalendarException("Erreur : La date de disponibilite est anterieure a la date d'echeance");
    nom=n;
    disponibilite=dispo;
    echeance=deadline;
}

Projet::~Projet(){
    QMap <QString,Tache*>::iterator it=taches.begin();
    while (it!=taches.end()) {
        delete (*it);
        it=taches.erase(it);
    }
}

void Projet::setNom(const QString& name) {
    if (ProjetManager::getInstance().isProjetExistant(name)) throw CalendarException("erreur ProjetManager : projet "+name+" déjà existant");
    nom=name;
}

void Projet::addTache(Tache* t){
    if(t->getDateDisponibilite()<disponibilite || t->getDateEcheance()>echeance)
        throw CalendarException("Les dates du projet et de la tache sont en conflit.");
    QMap<QString, Tache*>::iterator it=taches.find(t->getId());
    if (it!=taches.end())
        throw CalendarException("La tache est deja dans le projet.");
    taches.insert(t->getId(), t);
}

Tache* Projet::trouverTache(const QString& id)const{
    QMap<QString, Tache*>::const_iterator it=taches.find(id);
    if (it!=taches.end()) return *it;
    else return 0;
}

const QMap<QString, Tache*> Projet::getTachesDispo(const QDate& d){
    QMap<QString, Tache*> resultat;
    for(QMap<QString, Tache*>::iterator it=taches.begin(); it!=taches.end(); it++) {
        if ((*it)->getDateDisponibilite()<=d && (*it)->getDateEcheance()>=d)
            resultat.insert((*it)->getId(), *it);
    }
    return resultat;
}

const QMap<QString, Tache*> Projet::getTachesComposites() const{
    QMap<QString, Tache*> resultat;
    for(QMap<QString, Tache*>::const_iterator it=taches.begin(); it!=taches.end(); it++) {
        if ((*it)->isComposite())
            resultat.insert((*it)->getId(), *it);
    }
    return resultat;
}

void Projet::verifAjoutTache(const QString& id, const QDate& dispo, const QDate& deadline, const TacheComposite* mere) {
    ProjetManager& pjm=ProjetManager::getInstance();
    if (pjm.isTacheExistante(id)) throw CalendarException("erreur, ProjetManager, tache"+id+" deja existante");
    if (disponibilite>dispo || echeance<deadline)
        throw CalendarException("erreur, Projet, creation de la tache impossible car les dates sont en conflit.");
    if (dispo>deadline)
        throw CalendarException("erreur, Projet, la date de disponibilite doit etre anterieure a la date d'echeance.");
    if (mere!=0 && mere->getProjet()!=this)
        throw CalendarException("erreur, Projet, creation de la tache impossible car la tache mere ne fait pas partie du meme projet.");

}

TacheUnitaire& Projet::ajouterTacheUnitaire(const QString& id, const QString& t, const QDate& dispo, const QDate& deadline, const Duree& dur, bool preempt, const TacheComposite* mere){
    verifAjoutTache(id,dispo,deadline,mere);
    if (dur.getDureeEnMinutes()<=0)
        throw CalendarException("erreur, Projet, creation de la tache impossible car la duree doit etre strictement positive");
    if (preempt==false && dur.getDureeEnHeures()>12)
        throw CalendarException("Une tache non-preemptive ne peut avoir une duree superieure à 12 heures");
    TacheUnitaire* newt=new TacheUnitaire(id,t,dispo,deadline,this,dur,mere,preempt);
    addTache(newt);
    return *newt;
}

TacheComposite& Projet::ajouterTacheComposite(const QString& id, const QString& t, const QDate& dispo, const QDate& deadline, const TacheComposite* mere){
    verifAjoutTache(id,dispo,deadline,mere);
    TacheComposite* newt=new TacheComposite(id,t,dispo,deadline,this);
    addTache(newt);
    return *newt;
}

 const QMap<QString,Tache*>::iterator &Projet::removeTache(Tache* t) {
    if (t->getProjet()==this) {

        if (t->isUnitaire()) { //Si c'est une tâche unitaire
            ProgrammationManager::getInstance().suppressionProgrammations(static_cast<TacheUnitaire*>(t)); //On supprime ses programamtions
        }
        else { //Si c'est une tâche composite
            static_cast<TacheComposite*>(t)->removeTacheDeMere(); //On l'enlève comme tache mère à ses sous-tâches
        }
        t->removeTacheDePred();
        t->removeTacheDeSuiv();
        if (t->getTacheMere()!=0) {
            static_cast<TacheComposite*>(trouverTache(t->getTacheMere()->getId()))->removeSousTache(t); // Si elle a une tâche mère on l'enlève des sous-tâche de la tâche mère.
        }
        QMap<QString,Tache*>::iterator it=taches.begin();
        while ((*it)!=t) {
            ++it;
        }
        delete t; //On supprime la tâche en appelant son destructeur en respectant le polymorphisme.
        return taches.erase(it);
    }
    else
        return taches.end();
}

/**************************************************************************************************************************************************/

    /**************************************/
    /*           PROJET MANAGER           */
    /**************************************/

ProjetManager::~ProjetManager(){
    QMap <QString,Projet*>::iterator it=projets.begin();
    while (it!=projets.end()) {
        delete (*it);
        it=projets.erase(it);
    }
    qDebug()<<"Destruction de ProjetManager\n";
}

Projet* ProjetManager::trouverProjet(const QString& name) const {
    QMap<QString, Projet*>::const_iterator it=projets.find(name);
    if (it!=projets.end()) return *it;
    else return 0;
}

ProjetManager::Handler ProjetManager::handler=ProjetManager::Handler();

ProjetManager& ProjetManager::getInstance(){
    if (handler.instance==0) handler.instance=new ProjetManager;
    return *(handler.instance);
}

void ProjetManager::libererInstance(){
    if (handler.instance!=0) delete handler.instance;
    handler.instance=0;
}

Projet& ProjetManager::ajouterProjet(const QString& name, const QDate& dispo, const QDate& deadline){
    if (trouverProjet(name)) throw CalendarException("erreur, ProjetManager, projet deja existante");
    Projet* newproj=new Projet(name,dispo,deadline);
    projets.insert(newproj->getNom(), newproj);
    return *newproj;
}

void ProjetManager::removeProjet(Projet *proj) {
    QMap<QString, Tache*>::iterator it=proj->taches.begin();
    while(it!=proj->taches.end()) {
        it=proj->removeTache(*it);
    }
    projets.remove(proj->getNom());
    delete proj;
}

Tache* ProjetManager::trouverTache(const QString& id)const {
    QMap<QString, Projet*>::const_iterator it=projets.begin();
    Tache* t=0;
    while (!t && it!=projets.end()) {
        t=(*it)->trouverTache(id);
        ++it;
    }
    return t;
}

bool ProjetManager::isTacheExistante(const QString& id) const{
    return (trouverTache(id)!=0);
}

/*void ProjetManager::removeTacheDePred(Tache *t){
    for (QMap<QString, Projet*>::iterator it=projets.begin(); it!=projets.end() ; ++it) {//On itère sur les projets...
        (*it)->removeTacheDePred(t);
    }
}*/

/**************************************************************************************************************************************************/

    /**************************************/
    /*           PROGRAMMATION            */
    /**************************************/

QTextStream& operator<<(QDataStream& f, const Programmation& p);

Programmation::~Programmation() {
    if (evenement!=0) delete evenement;
}

/**************************************************************************************************************************************************/

    /**************************************/
    /*       PROGRAMMATION MANAGER        */
    /**************************************/

ProgrammationManager::~ProgrammationManager() {
    QMap<QDateTime,Programmation*>::iterator it=programmations.begin();
    while(it!=programmations.end()){
        delete (*it);
        it=programmations.erase(it);
    }
    qDebug()<<"Destruction de ProgrammationManager\n";
}

ProgrammationManager::Handler ProgrammationManager::handler=ProgrammationManager::Handler();

ProgrammationManager& ProgrammationManager::getInstance() {
    if (handler.instance==0) handler.instance=new ProgrammationManager;
    return *(handler.instance);
}

void ProgrammationManager::libererInstance(){
    if (handler.instance!=0) delete handler.instance;
    handler.instance=0;
}

const Programmation* ProgrammationManager::trouverEvenement(const QString& name) {
    QMap<QDateTime,Programmation*>::const_iterator it=programmations.begin();
    while (it!=programmations.end()) {
        if ((*it)->getEvenement()!=0 && (*it)->getEvenement()->getNom()==name)
            return (*it);
        ++it;
    }
    return 0;
}

bool ProgrammationManager::plageHoraireDispo(const QDateTime& debut, const Duree& dur){
    bool resultat=true;
    QDateTime fin=debut+dur; // <! fin de la plage horaire demandée
    QDateTime progDebut; // <! début d'une programmation
    QDateTime progFin; // <! fin d'une programmation
    QMap<QDateTime,Programmation*>::const_iterator it=programmations.begin(); // <! On va regarder pour toutes les programmations
    while (it!=programmations.end() && resultat && (*it)->getDateTime()<=fin) { /* <! tant qu'il y en a et que la plage horaire est toujours disponible.
                                                                                    Les programamtions étant classées par date de début, à partir du moment où
                                                                                    l'on atteint une programmation qui commence après la fin de la plage horaire,
                                                                                    c'est que cette dernière est disponible */
        progDebut=(*it)->getDateTime(); // <! début de la programmation
        progFin=progDebut+(*it)->getDuree(); // <! fin de la programmation
        if (progDebut>=debut && progDebut<fin) resultat=false; // <! si le début de la programmation est pendant la plage horaire, celle-ci n'est donc pas disponible.
        if (progFin>debut && progFin<=fin) resultat=false; // <! de même pour la fin de la programmation.
        if (progDebut<debut && progFin>fin) resultat=false; // <! si la programmation commence avant et termine après la demande plage horaire.
        ++it;
    }
    return resultat; // <! Sera à true si la plage horaire est effectivement disponible
}

Programmation& ProgrammationManager::ajouterProgrammationTache(const TacheUnitaire* t, const QDateTime& dt, const Duree& dur){
    if(dt.date()<t->getDateDisponibilite() || dt.date()>t->getDateEcheance())
        throw CalendarException("La date indiquée ne figure pas entre la disponibilite et l'echeance de la tache.");
    if ((dt+dur).date()>t->getDateEcheance())
        throw CalendarException("La duree donne en argument rentre en conflit avec l'echeance de la tache pour la date et l'horaire indiquées en arguments");
    if(!(t->isPreemptive()) && t->getDuree()!=dur)
        throw CalendarException("La tache est non-preemptive donc la duree de programmation doit etre egale a celle donnee en argument.");
    if(t->isProgramee())
        throw CalendarException("La tache est deja programée");
    if(t->dureeRestanteAProgrammer()<dur)
        throw CalendarException("La duree restante a programmer < dur");
    if(!(plageHoraireDispo(dt, dur)))
        throw CalendarException("La plage horaire n'est pas disponible.");
    if(!(t->arePrecedencesProgrammeesAvant(dt)))
        throw CalendarException("Au moins une tache precedente a la tache n'est pas programmee");
    Programmation* newprog=new Programmation(t,dt,dur);
    programmations.insert(dt, newprog);
    return *newprog;
}

Programmation& ProgrammationManager::ajouterProgrammationEvenement(const QString& name, const QString& tp, const QString& desc, const QString& where, const QDateTime &dt, const Duree& dur) {
    if (trouverEvenement(name)!=0)
        throw CalendarException("Un evenement "+name+" existe deja");
    if (dur.getDureeEnMinutes()<=0)
        throw CalendarException("La duree de l'evenement doit etre strictement positive");
    if(!(plageHoraireDispo(dt, dur)))
        throw CalendarException("La plage horaire n'est pas disponible.");
    Programmation* newprog = new Programmation(name,tp,desc,where,dt,dur);
    programmations.insert(dt, newprog);
    return *newprog;
}

void ProgrammationManager::suppressionProgrammations(const TacheUnitaire* t) {
    QMap<QDateTime, Programmation*>::iterator it=programmations.begin(); // <! On va itérer sur les programmations
    while (it!=programmations.end()) { // <! Pour les programmations se trouvant donc entre la dispo et l'echeance de la tache
        if ((*it)->getTache()==t) { // <! Si la programmation concerne la tache.
            delete (*it); // <! On la supprime.
            it=programmations.erase(it);
            continue;
        }
        ++it;
    }
}

QMap<QDateTime,Programmation*> ProgrammationManager::getProgrammations(const TacheUnitaire* t) const{
    QMap<QDateTime,Programmation*> resultat;
    QMap<QDateTime,Programmation*>::const_iterator it=programmations.begin();
    while (it!=programmations.end() && (*it)->getDate()<=t->getDateEcheance()) {
        if ((*it)->getTache()==t)
            resultat.insert((*it)->getDateTime(), *it);
        ++it;
    }
    return resultat;
}

/*Déprogrammation*/

void ProgrammationManager::deprogrammationEvenement(const QDateTime& dt) {
    QMap<QDateTime,Programmation*>::iterator it=programmations.find(dt);
    if (it!=programmations.end() && (*it)->getEvenement()!=0) {
        delete (*it);
        programmations.erase(it);
    }
}

void ProgrammationManager::deprogrammationTotale(const TacheComposite* t) {
    deprogrammationSuiv(t);
    for (QMap<QString,Tache*>::const_iterator it=t->getSousTaches().begin(); it!=t->getSousTaches().end(); it++) {
        if ((*it)->isUnitaire())
            deprogrammationTotale(static_cast<TacheUnitaire*>(*it));
        else
            deprogrammationTotale(static_cast<TacheComposite*>(*it));
    }
}

void ProgrammationManager::deprogrammationTotale(const TacheUnitaire* t) {
    deprogrammationSuiv(t);
    suppressionProgrammations(t);
}

void ProgrammationManager::deprogrammationPartielle(const TacheUnitaire* t, const QDateTime& dt) {
    deprogrammationSuiv(t);
    QMap<QDateTime, Programmation*>::iterator it=programmations.begin(); // <! On va itérer sur les programmations
    while (it!=programmations.end()) { // <! Si la date et l'horaire existent et qu'elles concernent bien la tache
        if ((*it)->getTache()==t && dt==((*it)->getDateTime())) {
            delete (*it); // <! On supprime la programmation.
            it=programmations.erase(it);
            break;
        }
        ++it;
    }
}

void ProgrammationManager::deprogrammationSuiv(const Tache* t) {
    for (QMap<QString,Tache*>::const_iterator it=t->getSuiv().begin(); it!=t->getSuiv().end(); it++) {
        if ((*it)->isUnitaire())
            deprogrammationTotale(static_cast<TacheUnitaire*>(*it));
        else
            deprogrammationTotale(static_cast<TacheComposite*>(*it));
    }
    if (t->getTacheMere()!=0)
        deprogrammationSuiv(t->getTacheMere());
}

/**************************************************************************************************************************************************/

    /**************************************/
    /*           SAVE & LOAD              */
    /**************************************/

void save(const QString& f) {
    QFile newfile(f);
    if (!newfile.open(QIODevice::WriteOnly | QIODevice::Text))
        throw CalendarException(QString("erreur sauvegarde tâches : ouverture fichier xml"));
    QXmlStreamWriter stream(&newfile);
    stream.setAutoFormatting(true);
    stream.writeStartDocument();
    stream.writeStartElement("calendrier");
    ProjetManager& pjm=ProjetManager::getInstance();
    stream.writeStartElement("projets");
    for(QMap<QString,Projet*>::const_iterator it=pjm.getProjets().begin(); it!=pjm.getProjets().end(); it++){
        stream.writeStartElement("projet");
        stream.writeTextElement("nom",(*it)->getNom());
        stream.writeTextElement("disponibilite",(*it)->getDateDisponibilite().toString(Qt::ISODate));
        stream.writeTextElement("echeance",(*it)->getDateEcheance().toString(Qt::ISODate));
        stream.writeEndElement();
    }
    stream.writeEndElement();
    stream.writeStartElement("taches");
    for(QMap<QString,Projet*>::const_iterator it_proj=pjm.getProjets().begin(); it_proj!=pjm.getProjets().end(); it_proj++){
        for(QMap<QString,Tache*>::const_iterator it_tache=(*it_proj)->getTaches().begin(); it_tache!=(*it_proj)->getTaches().end(); it_tache++){
            stream.writeStartElement("tache");
            if ((*it_tache)->isUnitaire())
                stream.writeAttribute("preemptive", (static_cast<TacheUnitaire*>(*it_tache)->isPreemptive())?"true":"false");
            stream.writeTextElement("identificateur",(*it_tache)->getId());
            stream.writeTextElement("titre",(*it_tache)->getTitre());
            stream.writeTextElement("disponibilite",(*it_tache)->getDateDisponibilite().toString(Qt::ISODate));
            stream.writeTextElement("echeance",(*it_tache)->getDateEcheance().toString(Qt::ISODate));
            stream.writeTextElement("projet",(*it_tache)->getProjet()->getNom());
            if ((*it_tache)->isUnitaire()) {
                QString str;
                str.setNum((*it_tache)->getDuree().getDureeEnMinutes());
                stream.writeTextElement("duree",str);
            }
            stream.writeEndElement();
        }
    }
    stream.writeEndElement();
    stream.writeStartElement("precedences");
    for(QMap<QString,Projet*>::const_iterator it_proj=pjm.getProjets().begin(); it_proj!=pjm.getProjets().end(); it_proj++){
        for(QMap<QString,Tache*>::const_iterator it_tache=(*it_proj)->getTaches().begin(); it_tache!=(*it_proj)->getTaches().end(); it_tache++){
            if (!((*it_tache)->getPred().empty())) {
                stream.writeStartElement("precedence");
                stream.writeAttribute("tache", ((*it_tache)->getId()));
                for(QMap<QString,Tache*>::const_iterator it_pred=(*it_tache)->getPred().begin(); it_pred!=(*it_tache)->getPred().end(); it_pred++) {
                    stream.writeTextElement("tache",(*it_pred)->getId());
                }
                stream.writeEndElement();
            }
        }
    }
    stream.writeEndElement();
    stream.writeStartElement("sous-taches");
    for(QMap<QString,Projet*>::const_iterator it_proj=pjm.getProjets().begin(); it_proj!=pjm.getProjets().end(); it_proj++){
        const QMap<QString,Tache*> tachesComp=(*it_proj)->getTachesComposites();
        for(QMap<QString,Tache*>::const_iterator it_tache=tachesComp.begin(); it_tache!=tachesComp.end(); it_tache++){
            if (!(static_cast<TacheComposite*>(*it_tache)->getSousTaches().empty())) {
                stream.writeStartElement("sous-tache");
                stream.writeAttribute("tache", ((*it_tache)->getId()));
                QMap<QString,Tache*> soustaches=static_cast<TacheComposite*>(*it_tache)->getSousTaches();
                for(QMap<QString,Tache*>::const_iterator it_ss=soustaches.begin(); it_ss!=soustaches.end(); it_ss++) {
                    stream.writeTextElement("tache",(*it_ss)->getId());
                }
                stream.writeEndElement();
            }
        }
    }
    stream.writeEndElement();
    ProgrammationManager& pgm=ProgrammationManager::getInstance();
    stream.writeStartElement("programmations");
    for(QMap<QDateTime,Programmation*>::const_iterator it=pgm.getProgrammations().begin(); it!=pgm.getProgrammations().end(); it++){
        stream.writeStartElement("programmation");
        if ((*it)->getTache()!=0) {
            stream.writeTextElement("tache",(*it)->getTache()->getId());
        }
        else if ((*it)->getEvenement()!=0) {
            stream.writeStartElement("evenement");
            stream.writeTextElement("nom",(*it)->getEvenement()->getNom());
            stream.writeTextElement("type",(*it)->getEvenement()->getType());
            stream.writeTextElement("description",(*it)->getEvenement()->getDescription());
            stream.writeTextElement("lieu",(*it)->getEvenement()->getLieu());
            stream.writeEndElement();
        }
        stream.writeTextElement("dateTime",(*it)->getDateTime().toString(Qt::ISODate));
        QString str;
        str.setNum((*it)->getDuree().getDureeEnMinutes());
        stream.writeTextElement("duree",str);
        stream.writeEndElement();
    }
    stream.writeEndElement();
    stream.writeEndElement();
    stream.writeEndDocument();
    newfile.close();
}

QString file="";
void load(const QString& f){
    if (file!="") save(file);
    ProjetManager::libererInstance();
    ProgrammationManager::libererInstance();
    ProjetManager& pjm=ProjetManager::getInstance();
    ProgrammationManager& pgm=ProgrammationManager::getInstance();
    file=f;
    QFile fin(file);
    if (!fin.open(QIODevice::ReadOnly | QIODevice::Text)) {
        throw CalendarException("Erreur ouverture fichier");
    }
    QXmlStreamReader xml(&fin);
    QXmlStreamReader::TokenType  token;
    while(!xml.atEnd() && !xml.hasError()) {
        token = xml.readNext();
        if(token == QXmlStreamReader::StartDocument) { continue; }
        if(token == QXmlStreamReader::StartElement) {
            if(xml.name() == "projets") { //On crée d'abord les projets
                //qDebug()<<"Les projets";
                xml.readNext();
                while(!(xml.tokenType() == QXmlStreamReader::EndElement && xml.name() == "projets")) { //On va créer chaque projet
                    if (xml.name() == "projet") {
                        QString name;
                        QDate dispo;
                        QDate deadline;
                        //qDebug()<<"Projet";
                        xml.readNext();
                        while(!(xml.tokenType() == QXmlStreamReader::EndElement && xml.name() == "projet")) { //on lit tous les sous-éléments du projet
                            if(xml.tokenType() == QXmlStreamReader::StartElement) {
                                if(xml.name() == "nom") {
                                    xml.readNext();
                                    name = xml.text().toString();
                                }
                                else if(xml.name() == "disponibilite") {
                                    xml.readNext();
                                    dispo = QDate::fromString(xml.text().toString(),Qt::ISODate);
                                }
                                else if(xml.name() == "echeance") {
                                    xml.readNext();
                                    deadline = QDate::fromString(xml.text().toString(),Qt::ISODate);
                                }
                            }
                            xml.readNext();
                        }
                        pjm.ajouterProjet(name,dispo,deadline);
                    }
                    xml.readNext();
                }
                continue;
            }
            if(xml.name() == "taches") { //On crée ensuite les tâches
                //qDebug()<<"Les taches";
                xml.readNext();
                while(!(xml.tokenType() == QXmlStreamReader::EndElement && xml.name() == "taches")) { //On va créer chaque tâche
                    if (xml.tokenType() == QXmlStreamReader::StartElement && xml.name() == "tache") {
                        QString id;
                        QString t;
                        QDate dispo;
                        QDate deadline;
                        Projet* proj;
                        Duree dur;
                        bool preempt;
                        bool unitaire = false;
                        QXmlStreamAttributes attributes = xml.attributes();
                        if(attributes.hasAttribute("preemptive")) {
                            unitaire = true;
                            preempt=(attributes.value("preemptive").toString() == "true" ? true : false);
                        }
                        xml.readNext();
                        while(!(xml.tokenType() == QXmlStreamReader::EndElement && xml.name() == "tache")) { //On s'occupe d'une tâche

                            if(xml.tokenType() == QXmlStreamReader::StartElement) {
                                if(xml.name() == "identificateur") {
                                    xml.readNext();
                                    id = xml.text().toString();
                                }
                                if(xml.name() == "titre") {
                                    xml.readNext();
                                    t = xml.text().toString();
                                }
                                if(xml.name() == "projet") {
                                    xml.readNext();
                                    proj = pjm.trouverProjet(xml.text().toString());
                                }
                                if(xml.name() == "disponibilite") {
                                    xml.readNext();
                                    dispo = QDate::fromString(xml.text().toString(),Qt::ISODate);
                                }
                                if(xml.name() == "echeance") {
                                    xml.readNext();
                                    deadline = QDate::fromString(xml.text().toString(),Qt::ISODate);
                                }
                                if(xml.name() == "duree") {
                                    xml.readNext();
                                    dur.setDuree(xml.text().toString().toUInt());
                                }
                            }
                            xml.readNext();
                        }
                        Tache* tache;
                        if (proj != 0){

                            if (unitaire)
                                tache = &proj->ajouterTacheUnitaire(id,t,dispo,deadline,dur,preempt);
                            else
                                tache = &proj->ajouterTacheComposite(id,t,dispo,deadline);
                        }
                        proj=0;
                    }
                    xml.readNext();
                }
                continue;
            }
            if (xml.name() == "precedences") { //On crée chaque précédence
                //qDebug()<<"Les precedences";
                xml.readNext();
                while(!(xml.tokenType() == QXmlStreamReader::EndElement && xml.name() == "precedences")) {
                    xml.readNext();
                    if (xml.name() == "precedence") {
                        while(!(xml.tokenType() == QXmlStreamReader::EndElement && xml.name() == "precedence")) { //On s'occupe de la precedence de chacune des tâches.
                            Tache* tache;
                            QXmlStreamAttributes attributes = xml.attributes();
                            if(attributes.hasAttribute("tache")) {
                                tache=pjm.trouverTache(attributes.value("tache").toString());
                            }
                            xml.readNext();
                            if(xml.tokenType() == QXmlStreamReader::StartElement) {
                                Tache* pred;
                                if(xml.name() == "tache") {
                                    xml.readNext();
                                     pred=pjm.trouverTache(xml.text().toString());
                                }
                                tache->ajouterPrecedence(*pred);
                            }
                        }
                    }
                }
                continue;
            }
            if (xml.name() == "sous-taches") { //On rajoute les sous-taches aux taches composites
                //qDebug()<<"Les sous-taches";
                xml.readNext();
                while(!(xml.tokenType() == QXmlStreamReader::EndElement && xml.name() == "sous-taches")) {
                    if (xml.name() == "sous-tache") {
                        TacheComposite* tache;
                        while(!(xml.tokenType() == QXmlStreamReader::EndElement && xml.name() == "sous-tache")) { //On s'occupe de chacune des tâches composites
                            QXmlStreamAttributes attributes = xml.attributes();
                            if(attributes.hasAttribute("tache")) {
                                tache=static_cast<TacheComposite*>(pjm.trouverTache(attributes.value("tache").toString()));
                            }
                            xml.readNext();
                            if(xml.tokenType() == QXmlStreamReader::StartElement) {
                                Tache* soustache;
                                if(xml.name() == "tache") {
                                    xml.readNext();
                                    soustache=pjm.trouverTache(xml.text().toString());
                                    //qDebug()<<soustache->getId();
                                }
                                tache->ajouterSousTache(*soustache);
                            }
                        }
                    }
                    xml.readNext();
                }
                continue;
            }
            if(xml.name() == "programmations") { //On passe à présent aux programmations
                //qDebug()<<"Les programamtions";
                xml.readNext();
                while(!(xml.tokenType() == QXmlStreamReader::EndElement && xml.name() == "programmations")) {
                    if(xml.tokenType() == QXmlStreamReader::StartElement && xml.name() == "programmation") {
                        bool evenement=false;
                        TacheUnitaire* tache;
                        QString n;
                        QString t;
                        QString d;
                        QString l;
                        QDateTime dt;
                        Duree dur;

                        xml.readNext();

                        while(!(xml.tokenType() == QXmlStreamReader::EndElement && xml.name() == "programmation")) { //On s'occupe de chaque programamtion
                            if(xml.tokenType() == QXmlStreamReader::StartElement) {
                                if(xml.name() == "evenement"){ //Si elle concerne un evenement
                                    evenement=true;
                                    xml.readNext();
                                    while(!(xml.tokenType() == QXmlStreamReader::EndElement && xml.name() == "evenement")) {
                                        if (xml.tokenType() == QXmlStreamReader::StartElement) {
                                            if(xml.name() == "nom") {
                                                xml.readNext();
                                                n = xml.text().toString();
                                            }
                                            if(xml.name() == "type") {
                                                xml.readNext();
                                                t = xml.text().toString();
                                            }
                                            if(xml.name() == "description") {
                                                xml.readNext();
                                                d = xml.text().toString();
                                            }
                                            if(xml.name() == "lieu") {
                                                xml.readNext();
                                                l = xml.text().toString();
                                            }
                                        }
                                        xml.readNext();
                                    }
                                }
                                if(xml.name() == "tache"){ //Si elle concerne une tâche
                                    evenement=false;
                                    xml.readNext();
                                    tache = tache=static_cast<TacheUnitaire*>(pjm.trouverTache(xml.text().toString()));
                                }
                                if(xml.name() == "dateTime"){
                                    xml.readNext();
                                    dt = QDateTime::fromString(xml.text().toString(),Qt::ISODate);
                                }
                                if(xml.name() == "duree"){
                                    xml.readNext();
                                    dur.setDuree(xml.text().toString().toUInt());
                                }
                            }
                            xml.readNext();
                        }
                        if (evenement){
                            pgm.ajouterProgrammationEvenement(n,t,d,l,dt,dur);
                            evenement=false;
                        }
                        else{
                            pgm.ajouterProgrammationTache(tache,dt,dur);
                        }
                    }
                    xml.readNext();
                }
                continue;
            }
        }
    }
    if(xml.hasError()) throw CalendarException(xml.errorString());
    xml.clear();
}

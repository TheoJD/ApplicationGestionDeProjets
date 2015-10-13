#ifndef CALENDAR_h
#define CALENDAR_h
#include <QString>
#include <QDate>
#include <QTextStream>
#include <QMap>
#include <QMessageBox>
#include <QDateTime>
#include <QFile>
#include <QTextCodec>
#include <QtXml>
#include <QFile>

/**
 * \file Calendar.h
 * \brief Partie back-end du projet contenant toute l'architecture des Projet, Tache, Evenement et Programmation.
 */

using namespace std;

void save(const QString& f); /*!< Permet de sauvegarder sous format XML un calendrier dans un QFile */
void load(const QString& f); /*!< Permet de charger depuis un QFile f .xml un calendrier */

    /**************************************/
    /*         CALENDAR EXCEPTION         */
    /**************************************/

/*! \class CalendarException
        \brief Classe permettant la récupération d'informations sur les erreurs d'ajout, de suppression etc.
*/
class CalendarException{
public:
    CalendarException(const QString& message):info(message){}
    QString getInfo() const { return info; }
private:
    QString info;
};

/************************************************************************************************************************************************/

    /**************************************/
    /*               DUREE                */
    /**************************************/

/*! \class Duree
        \brief Classe permettant de manipuler des durees
        L'utilisation de cette classe n?cessite des dates valides au sens commun du terme.
        D?clenchement d'exception dans le cas contraire
*/
class Duree{
public:
    //! Constructeur ? partir de heure et minute
    /*! \param h heure avec h>=0
        \param m minute avec 0<=m<=59
        */
    Duree(unsigned int h, unsigned int m):nb_minutes(h*60+m) {if (m>59) throw CalendarException("erreur: initialisation duree invalide");}
    //! Constructeur ? partir de minute
    /*! \param m minute avec m>=0
        */
    Duree(unsigned int m=0):nb_minutes(m) {}
    void setDuree(unsigned int minutes) { nb_minutes=minutes; }
    void setDuree(unsigned int heures, unsigned int minutes) { if (heures==0 && minutes>59) throw CalendarException("erreur: initialisation duree invalide"); nb_minutes=heures*60+minutes; }
    unsigned int getDureeEnSecondes() const { return nb_minutes*60; } //<!Retourne la duree en secondes
    unsigned int getDureeEnMinutes() const { return nb_minutes; } //<!Retourne la duree en minutes
    double getDureeEnHeures() const { return double(nb_minutes)/60; } //<!Retourne la duree en heures
    unsigned int getMinute() const { return nb_minutes%60; }
    unsigned int getHeure() const { return nb_minutes/60; }
    QString stringDuree() const;
    void afficher(QTextStream& f) const; //<!Affiche la duree sous le format hhHmm
private:
    unsigned int nb_minutes;
};

QTextStream& operator<<(QTextStream& f, const Duree & d);
QTextStream& operator>>(QTextStream&, Duree&); //lecture format hhHmm
Duree operator+(const Duree& d1, const Duree& d2);
Duree operator-(const Duree& d1, const Duree& d2);
Duree operator+=(Duree& d1, const Duree& d2);
bool operator<=(const Duree& d1, const Duree& d2);
bool operator>=(const Duree& d1, const Duree& d2);
bool operator<(const Duree& d1, const Duree& d2);
bool operator>(const Duree& d1, const Duree& d2);
bool operator==(const Duree& d1, const Duree& d2);
bool operator!=(const Duree& d1, const Duree& d2);
QDateTime operator+(const QDateTime& d, const Duree& dur);

/************************************************************************************************************************************************/

    /**************************************/
    /*               TACHE                */
    /**************************************/

class TacheComposite;
class Projet;


/*! \class Tache
        \brief Classe permettant de manipuler des tâches.
        Cette classe est abstraite, on ne l'instanciera que par l'intermédiaire de ses classes filles.
        Les dates doivent être cohérentes, une tâche doit appartenir à un Projet, mais peut ne pas avoir de TacheComposite mère.
*/
class Tache {
    friend class Projet;
    friend void save(const QString& f);
    QString identificateur;
    QString titre;
    QDate disponibilite;
    QDate echeance;
    QMap<QString, Tache*> pred; /*!< Tableau dynamique contenant des pointeurs vers les tâches précédentes, triées par ID*/
    QMap<QString, Tache*> suiv; /*!< Tableau dynamique contenant des pointeurs vers les tâches suivantes, triées par ID*/
    bool unitaire;
    const Projet* projet;
    const TacheComposite* tacheMere;
    Tache(const Tache& t); /*!< Privé pour empêcher la recopie*/
    Tache& operator=(const Tache&); /*!< Privé pour empêcher la recopie*/
    void setDatesDisponibiliteEcheance(const QDate& disp, const QDate& e) {
        if (e<disp) throw CalendarException("erreur Tache : date echeance < date disponibilite");
        //if (e>projet->getDateEcheance()) throw CalendarException("erreur Tache : date echeance incohérente avec le projet");
        //if (disp<projet->getDateDisponibilite()) throw CalendarException("erreur Tache : date disponiblite incohérente avec le projet");
        disponibilite=disp; echeance=e;
    } /*!< Change les dates si elles sont cohérentes entre elles et avec le projet*/

    void ajouterSuivant(Tache *t); /*!< Ajoute un suivant. Méthode privée car se fait en trigger avec l'ajout de précédence par la tâche en argument*/
    void removeTacheDePred(); /*!< Enlève la tâche des précédences des tâches qu'elle précède directement*/
    void removeTacheDeSuiv(); /*!< Enlève la tâche des suivants des tâches qu'elle suit directement*/
protected :
    /*!
         *  \brief Constructeur
         *
         *  Constructeur de la classe Tache
         *  Pas publique car seuls les projets doivent instancier des tâches.
         *
         *  \param id : identificateur (doit être unique)
         *  \param t : titre
         *  \param dispo : date de disponibilite
         *  \param dealine : date d'echeance
         *  \param proj : pointeur sur le Projet de la tâche
         *  \param u : true si la Tache est unitaire, false sinon
         *  \param mere : pointeur sur la TacheComposite mère, mis à 0 par défaut
         */
    Tache(const QString& id, const QString& t, const QDate& dispo, const QDate& deadline, const Projet* proj, const bool& u, const TacheComposite* mere=0):
        identificateur(id),titre(t),disponibilite(dispo),echeance(deadline), projet(proj), unitaire(u), tacheMere(mere){}
    /*!
         *  \brief Destructeur
         *
         *  Destructeur de la classe Tache
         *  Privé car le cycle de vie est géré par le projet de chaque tâche.
         *  Virtuel pour respecter le polymorphisme.
         */
    virtual ~Tache();
    virtual void verifAjoutPrecedence(const Tache& t) const; /*!< Vérifie toutes les contraintes de bases pour l'ajout de précédence. Virtuel pour ajouter des contraintes nouvelles comme pour les tâches composites. */
public:
    //Attributs
    const QString& getId() const { return identificateur; } /*!< Accesseur à l'identificateur en lecture*/
    void setId(const QString& str); /*!< Si le nouvel id n'est pas déjà utilisé, change l'id*/
    const QString& getTitre() const { return titre; } /*!< Accesseur au titre en lecture*/
    void setTitre(const QString& id) { titre=id; } /*!< Change le titre*/
    virtual Duree getDuree() const =0; /*!< Accesseur à la durée en lecture, méthode virtuelle pure */
    const QDate& getDateDisponibilite() const {  return disponibilite; } /*!< Accesseur à la date de disponibilité en lecture*/
    const QDate& getDateEcheance() const {  return echeance; } /*!< Accesseur à la date d'échéance en lecture*/
    const bool& isUnitaire() const { return unitaire; } /*!< True si la tâche est unitaire*/
    bool isComposite() const { return !unitaire; } /*!< True si la tâche est composite*/
    const Projet* getProjet() const { return projet; } /*!< Accesseur au projet en lecture */
    const TacheComposite* getTacheMere()  const { return tacheMere; } /*!< Accesseur à la tâche mère en lecture */
    virtual void setTacheMere(const TacheComposite* mere); /*!< Change la tâche mère si elles sont du même projet, indépendantes l'une de l'autre, avec des dates cohérentes. Virtuelle */
    void removeTacheMere(); /*!< Supprime le pointeur vers la tâche mère */
    const QMap<QString, Tache*>& getPred() const { return pred; } /*!< Accesseur au tableau de précédences en lecture */
    const QMap<QString, Tache*>& getSuiv() const { return suiv; } /*!< Accesseur au tableau de suivants en lecture */
    //Precedences
    bool hasAlreadyPrecedence(const Tache* t) const; /*!< True si déjà précédée directement au indirectement par une tâche */
    bool isPrecedentDe(const Tache* t) const; /*!< True si précède déjà directement au indirectement une tâche */
    void ajouterPrecedence(Tache &t); /*!< Ajoute une tâche en précédence si les conditions sont respectées */
    int removePrecedence(Tache* t) {  return pred.remove(t->getId()); } /*!< Supprime simplement une précédence */
    void afficherPrecedences() const;
    //Suivants
    bool hasSuivant(const Tache* t) const; /*!< True si déjà suivie directement au indirectement par une tâche */
    bool isSuivantDe(const Tache* t) const; /*!< True si suit déjà directement au indirectement une tâche */
    int removeSuivant(const Tache* t) { return suiv.remove(t->getId()); } /*!< Supprime simplement un suivant */
    void afficherSuivant() const;
    //Sous-tache ?
    bool isSousTache(const TacheComposite *t) const; /*!< True si la tâche composite est sa mère ou un ancêtre de sa mère */
    //Programmation
    virtual bool isProgrameeAvant(const QDateTime& dt) const = 0; /*!< True si programmée entièrement avant une QDateTime précise. Virtuelle pure */
    virtual bool isProgramee() const = 0; /*!< True si programmée entièrement. Virtuelle pure */
    bool arePrecedencesProgrammeesAvant(const QDateTime &dt) const; /*!< True si toutes les précédences sont programmées avant */
};

QTextStream& operator<<(QTextStream& f, const Tache& t);

/************************************************************************************************************************************************/

    /**************************************/
    /*           TACHE UNITAIRE           */
    /**************************************/

/*! \class TacheUnitaire
        \brief Classe permettant de manipuler des TacheUnitaire.
        Les Projet instancient cette classe, pas l'utilisateur directement.
        Ces Tache sont programmables, doivent avoir une durée positive et se distinguent suivant qu'elles sont préemptives ou non.
*/
class TacheUnitaire : public Tache {
    friend class Projet;
    Duree duree;
    bool preemptive;
protected :
    /*!
         *  \brief Constructeur
         *
         *  Constructeur de la classe TacheUnitaire
         *  Protected car seuls les projets doivent instancier des Tache.
         *
         *  \param dur : doit être positive
         *  \param preempt : false par défaut, indique si la tâche peut être ou non programmée en plusieurs fois.
         */
    TacheUnitaire(const QString& id,const QString& t,const QDate& dispo,const QDate& deadline, const Projet* proj, const Duree& dur, const TacheComposite* mere=0, bool preempt=false) :
        Tache(id, t, dispo, deadline, proj, true, mere), duree(dur), preemptive(preempt) {}
    /*!
         *  \brief Destructeur
         *
         *  Destructeur de la classe TacheUnitaire
         *  Protected car le cycle de vie est géré par le Projet de chaque Tache.
         *  Virtuel si jamais il y a héritage un jour.
         */
    virtual ~TacheUnitaire();
public :
    //Attributs
    Duree getDuree() const { return duree; } /*!< Accesseur en lecture à l'attribut durée */
    void setDuree(const Duree& d) { if (d>Duree(0)) duree=d; } /*!< Modifie la durée si elle est positive */
    const bool& isPreemptive() const { return preemptive; } /*!< True si la tâche est programmable en plusieurs fois */
    void setPreemptive() {  preemptive=true; } /*!< Rend la tâche préemptive  */
    void setNonPreemptive() { if (duree.getDureeEnHeures()<=12) preemptive=false; } /*!< Si la durée est inférieure à 12h, rend la tâche non-préemptive */
    void verifAjoutPrecedence(const Tache &t) const;
    //Programmation
    bool isProgramee() const; /*!< True si la tâche est entièrement programmée  */
    bool isProgrameeAvant(const QDateTime& dt) const; /*!< True si la tâche est entièrement programmée avant une QDateTime précise */
    Duree dureeRestanteAProgrammer() const; /*!< Retourne la différence entre la durée de la TacheUnitaire et la durée des Programmation de la TacheUnitaire */
};

/************************************************************************************************************************************************/

    /**************************************/
    /*           TACHE COMPOSITE          */
    /**************************************/

/*! \class TacheComposite
        \brief Classe permettant de manipuler des TacheComposite.
        Les Projet instancient cette classe, pas l'utilisateur directement.
        Une TacheComposite n'est pas programmable mais peut contenir des sous-tâches, TacheUnitaire comme TacheComposite.
*/
class TacheComposite : public Tache {
    friend class Projet;
    QMap<QString, Tache*> sousTaches;
protected :
    /*!
         *  \brief Constructeur
         *
         *  Constructeur de la classe TacheComposite.
         *  Protected car seuls les projets doivent instancier des tâches.
         */
    TacheComposite(const QString& id,const QString& t,const QDate& dispo,const QDate& deadline, const Projet* proj, const TacheComposite* mere=0) :
        Tache(id, t, dispo, deadline, proj, false, mere) {}
    /*!
         *  \brief Destructeur
         *
         *  Destructeur de la classe TacheComposite.
         *  Protected car le cycle de vie est géré par le Projet de chaque Tache.
         *  Virtuel si jamais il y a héritage un jour.
         */
    virtual ~TacheComposite();
public :
    //Attributs
    void setTacheMere(const TacheComposite* mere); /*!< Teste en plus s'il n'y a déjà une relation de sous-tâche dans l'autre sens */
    Duree getDuree() const; /*!< Retourne la somme de la durée des sous-tâches */
    const QMap<QString, Tache*>& getSousTaches() const { return sousTaches; } /*!< Accesseur en lecture aux sous-tâches */
    //Sous-taches
    bool hasAlreadySousTache(const Tache* t) const; /*!< True si déjà mère directement au indirectement d'une tâche */
    void ajouterSousTache(Tache& t); /*!< Si les conditions sont réunies, ajoute une sous-tâche */
    int removeSousTache(Tache* t) { t->removeTacheMere(); return sousTaches.remove(t->getId()); } /*!< Supprime les liens dans les deux sens avec une sous-tâche */
    void afficherSousTaches() const;
    void removeTacheDeMere(); /*!< Supprime la présence de la tache mère dans toutes les sous-tâches */
    //Precedence
    virtual void verifAjoutPrecedence(const Tache& t) const; /*!< Vérifie en plus des contraintes par rapport aux sous-tâches. Virtuel si une sous-classe fait naître de nouvelles contraintes. */
    //Programmation
    bool isProgramee() const; /*!< True si toutes les sous-tâches sont programmées */
    bool isProgrameeAvant(const QDateTime& dt) const; /*!< True si toutes les sous-tâches sont programmées avant une QDateTime précise */
};

/************************************************************************************************************************************************/

    /**************************************/
    /*               PROJET               */
    /**************************************/

/*! \class Projet
        \brief Classe permettant de manipuler des projets et des Tache.
        ProjetManager instancie les projets.
        Les dates du projet doivent être cohérentes, celles de ses tâches inclusent dans l'intervalle de réalisation du projet.
*/
class Projet {
    friend class ProjetManager;
    QString nom;
    QDate disponibilite;
    QDate echeance;
    QMap<QString, Tache*> taches; /*!< Tableau dynamique contenant les pointeurs vers les différentes tâches du projet */
    Projet(const Projet& p); /*!< Privé pour empêcher la recopie */
    Projet& operator=(const Projet& p); /*!< Privé pour empêcher la recopie */
    void addTache(Tache* t); /*!< Insère la tâche dans le tableau des Tache */
    void verifAjoutTache(const QString& id, const QDate& dispo, const QDate& deadline, const TacheComposite* mere); /*!< Vérifie les conditions d'ajout d'une Tache, la cohérence des dates par exemple */
protected :
    /*!
         *  \brief Constructeur
         *
         *  Constructeur de la classe Projet.
         *  Pas publique car seuls les projets doivent instancier des Tache.
         *  \param n : nom du projet. Doit être unique.
         *  \param dispo : date de disponibilité du projet
         *  \param deadline : date d'échéance du projet
         */
    Projet(const QString& n, const QDate& dispo, const QDate& deadline);
    /*!
         *  \brief Destructeur
         *
         *  Destructeur de la classe Projet.
         *  Protected car le cycle de vie est géré par ProjetManager.
         *  Détruit toutes les Tache du Projet.
         *  Virtuel si jamais il y a héritage un jour.
         */
    virtual ~Projet();
public:
    const QString& getNom() const { return nom; } /*!< Accesseur en lecture au nom */
    void setNom(const QString& name); /*!< Change le nom du projet s'il n'est pas déjà utilisé */
    const QDate& getDateDisponibilite() const {  return disponibilite; } /*!< Accesseur en lecture à la date de disponibilité */
    const QDate& getDateEcheance() const {  return echeance; } /*!< Accesseur en lecture à la date d'échéance */
    /*void setDatesDisponibiliteEcheance(const QDate& disp, const QDate& e) {
        if (e<disp) throw CalendarException("erreur Projet : date echeance < date disponibilite");
        disponibilite=disp; echeance=e;
    }*/
    const QMap<QString, Tache*>& getTaches() { return taches; } /*!< Accesseur en lecture aux tâches */
    Tache* trouverTache(const QString& id) const; /*!< Retourne 0 si une tâche ne fait pas partie du projet, le pointeur vers la tâche sinon */
    bool isTacheExistante(const QString& id) const { return trouverTache(id)!=0; } /*!< True si la tâche fait partie du projet */
    const QMap<QString, Tache*> getTachesDispo(const QDate& d); /*!< Renvoie le tableau des tâches disponibles à une QDate précise */
    const QMap<QString, Tache*> getTachesComposites() const; /*!< Liste de toutes les TacheComposite du projet */
    /*!
         *  \brief Ajout d'une TacheUnitaire
         *
         *  Methode qui permet de créer et ajouter une TacheUnitaire au Projet, si les conditions sont réunies.
         *  Les paramètres sont ceux du constructeur des TacheUnitaire.
         *
         *  \return une référence sur la TacheUnitaire crée.
         */
    TacheUnitaire& ajouterTacheUnitaire(const QString& id, const QString& t, const QDate& dispo, const QDate& deadline, const Duree& dur,
                                         bool preempt=false, const TacheComposite* mere=0);
    /*!
         *  \brief Ajout d'une TacheComposite
         *
         *  Methode qui permet de créer et ajouter une TacheComposite au Projet, si les conditions sont réunies.
         *  Les paramètres sont ceux du constructeur des TacheComposite.
         *
         *  \return une référence sur la TacheComposite crée.
         */
    TacheComposite& ajouterTacheComposite(const QString& id, const QString& t, const QDate& dispo, const QDate& deadline, const TacheComposite* mere=0);
    /*!
         *  \brief Suppression d'une tâche
         *
         *  Methode qui permet de supprimer une Tache, en s'assurant de la maintenance de cohérence des données.
         *  On supprime ses programmations sans supprimer les programmations des Tache suivantes.
         *  On supprime la Tache de toutes ses relations avec d'autres Tache.
         *  Enfin on la détruit.
         *
         *  \param t : pointeur sur la Tache.
         *  \return une référence sur la TacheComposite crée.
         */
    const QMap<QString,Tache*>::iterator &removeTache(Tache* t);
};

/**************************************************************************************************************************************************/

    /**************************************/
    /*           PROJET MANAGER           */
    /**************************************/

/*! \class ProjetManager
        \brief Classe permettant de créer et gérer l'ensemble des projets.
        C'est un singleton.
        Toutes ls tâches et projets sont accessibles depuis ProjetManager.
*/
class ProjetManager {
    friend class Projet;
    QMap<QString, Projet*> projets; /*!< Tableau dynamique contenant des pointeurs sur les Projet classés alphabétiquement. */
    /*!
         *  \brief Constructeur
         *
         *  Constructeur de la classe ProjetManager.
         *  Privé car singleton.
         */
    ProjetManager(){}
    /*!
         *  \brief Destructeur
         *
         *  Destructeur de la classe ProjetManager.
         *  Détruit tous les Projet qu'il contient.
         *  Privé car utilisé par l'instance statique.
         */
    ~ProjetManager();
    ProjetManager(const ProjetManager& um); /*!< Privé pour empêcher la recopie */
    ProjetManager& operator=(const ProjetManager& um); /*!< Privé pour empêcher la recopie */
    struct Handler{
        ProjetManager* instance;
        Handler():instance(0){}
        // destructeur appelé à la fin du programme
        ~Handler(){ if (instance!=0) delete instance;
                  qDebug()<<"Destruction de Handler\n";}
    }; /*!< Handler appliquant le design pattern Singleton */
    static Handler handler;
public :
    static ProjetManager& getInstance(); /*!< Retourne une référence sur l'objet ProjetManager, le créant si besoin. */
    static void libererInstance(); /*!< Détruit ProjetManager et réinitialise l'instance à 0 */
    const QMap<QString, Projet*>& getProjets(){return projets;} /*!< Accesseur en lecture aux Projet */
    Projet* trouverProjet(const QString& id) const; /*!< 0 si aucun Projet n'a ce nom, un pointeur sur le Projet sinon */
    bool isProjetExistant(const QString& name) { return (trouverProjet(name)!=0); } /*!< True si le Projet existe */
    Projet& ajouterProjet(const QString& name, const QDate& dispo, const QDate& deadline); /*!< Si le nom n'est pas déjà utilisé et les dates cohérentes, créé le projet. */
    void removeProjet(Projet* proj); /*!< Supprime le Projet en supprimant les Tache du Projet une à une pour maintenir la cohérence des données restantes */
    Tache* trouverTache(const QString& id) const; /*!< Pointe sur la Tache en regardant séquentiellement les Projet, vaut 0 sinon */
    bool isTacheExistante(const QString& id) const; /*!< True si une Tache avec cette ID existe dans un Projet. */
};

/**************************************************************************************************************************************************/

    /**************************************/
    /*             EVENEMENT              */
    /**************************************/

/*! \class Evenement
        \brief Classe permettant de manipuler des Evenement (ou activités).
        Un Evenement (une activité) n'existe que si il est programmé, donc son cycle de vie est lié à une Programmation.
        Deux évènements ont des noms distincts.
*/
class Evenement {
    friend class Programmation;
    QString nom;
    QString type;
    QString description;
    QString lieu;
    /*!
         *  \brief Constructeur
         *
         *  Constructeur de la classe Evenement (Activité).
         *  Privé car le cycle de vie est lié à la Programmation en charge.
         */
    Evenement(const QString& name, const QString& tp, const QString& desc, const QString& where) : nom(name), type(tp), description(desc), lieu(where) {}
    /*!
         *  \brief Destructeur
         *
         *  Destructeur de la classe Evenement (Activité).
         *  Privé car le cycle de vie est lié à la Programmation en charge.
         */
    virtual ~Evenement() {}
    Evenement(const Evenement& e); /*!< Privé pour empêcher la recopie */
    Evenement& operator=(const Evenement& e); /*!< Privé pour empêcher la recopie */
public :
    const QString& getNom() const { return nom; } /*!< Accesseur en lecture au nom */
    const QString& getType() const { return type; } /*!< Accesseur en lecture au type */
    void setType(const QString& tp) { type=tp; } /*!< Change le type de l'Evenement. */
    const QString& getDescription() const { return description; } /*!< Accesseur en lecture à la description */
    void setDescription(const QString& desc) { description=desc; } /*!< Change la description */
    const QString& getLieu() const { return lieu; } /*!< Accesseur en lecture au lieu */
    void setLieu (const QString& where) { lieu=where; } /*!< Change le lieu */
    void afficher() const {}
};

/**************************************************************************************************************************************************/

    /**************************************/
    /*           PROGRAMMATION            */
    /**************************************/

/*! \class Programmation
        \brief Classe permettant de manipuler des Programmation.
        Une Programmation concerne un Evenement ou une TacheUnitaire.
        Les propriétés d'une TacheUnitaire (durée, dates) doivent être cohérentes avec la programmation.
*/
class Programmation {
    friend class ProgrammationManager;
    const TacheUnitaire* tache; /*!< Pointe sur une TacheUnitaire mais ne peut la modifier. Vaut 0 si la Programmation concerne un Evenement */
    Evenement* evenement; /*!< Pointe sur un Evenement. Vaut 0 si la Programmation concerne une TacheUnitaire */
    QDateTime dateTime;
    Duree duree;
    Programmation(const Programmation& p); /*!< Privé pour empêcher la recopie */
    Programmation& operator=(const Programmation& p); /*!< Privé pour empêcher la recopie */
    /*!
         *  \brief Constructeur
         *
         *  Constructeur de la classe Programmation.
         *  Privé car le cycle de vie est lié à ProgrammationManager.
         *
         *  \param t : Pointeur sur la TacheUnitaire concernée
         *  \param dt : Date et heure de la Programmation
         *  \param dur : Durée de la Programmation
         */
    Programmation(const TacheUnitaire* t, const QDateTime& dt, const Duree& dur) : tache(t), evenement(0), dateTime(dt), duree(dur) {}
    /*!
         *  \brief Constructeur
         *
         *  Constructeur de la classe Programmation.
         *  Privé car le cycle de vie est lié à ProgrammationManager.
         *
         *  \param t : Pointeur sur la TacheUnitaire concernée
         *  \param d : Date de la Programmation
         *  \param h : Heure de la Programmation
         *  \param dur : Durée de la Programmation
         */
    Programmation(const TacheUnitaire* t, const QDate& d, const QTime& h, const Duree& dur) : tache(t), evenement(0), dateTime(QDateTime(d,h)), duree(dur) {}
    /*!
         *  \brief Constructeur
         *
         *  Constructeur de la classe Programmation.
         *  Privé car le cycle de vie est lié à ProgrammationManager.
         *
         *  Les premiers paramètres sont ceux du constructeur d'Evenement
         *
         *  \param dt : Date et heure de la Programmation
         *  \param dur : Durée de la Programmation
         */
    Programmation(const QString& name, const QString& tp, const QString& desc, const QString& where, const QDateTime& dt, const Duree& dur) : tache(0), dateTime(dt), duree(dur) {
        evenement = new Evenement(name,tp,desc,where);
    }
    /*!
         *  \brief Destructeur
         *
         *  Destructeur de la classe Programmation.
         *  Privé car le cycle de vie est lié à ProgrammationManager. Détruit l'Evenement si le pointeur est différent de 0.
         */
    virtual ~Programmation();
public:
    const TacheUnitaire* getTache() const { return tache; } /*!< Accesseur en lecture au pointeur sur la TacheUnitaire */
    const Evenement* getEvenement() const { return evenement; } /*!< Accesseur en lecture au pointeur sur l'Evenement */
    const QDateTime& getDateTime() const { return dateTime; } /*!< Accesseur en lecture à la date et l'heure */
    QDate getDate() const {  return dateTime.date(); } /*!< Accesseur en lecture à la date */
    QTime getTime() const  { return dateTime.time(); } /*!< Accesseur en lecture à l'heure */
    const Duree& getDuree() const  { return duree; } /*!< Accesseur en lecture à la durée */
};

/**************************************************************************************************************************************************/

    /**************************************/
    /*       PROGRAMMATION MANAGER        */
    /**************************************/

/*! \class ProgrammationManager
        \brief Classe permettant de créer et gérer des Programmation.
        C'est un singleton.
        Toutes les Programmation ont un cycle de vie liées à ProgrammationManager, elles sont toutes accessible à partir de cette classe.
*/
class ProgrammationManager {
    QMap<QDateTime, Programmation*> programmations;
    void addProgrammation(Programmation* prog);
    /*!
         *  \brief Constructeur
         *
         *  Constructeur de la classe ProjetManager.
         *  Privé car singleton.
         */
    ProgrammationManager() {}
    /*!
         *  \brief Destructeur
         *
         *  Destructeur de la classe ProjetManager.
         *  Détruit tous les Projet qu'il contient.
         *  Privé car utilisé par l'instance statique.
         */
    ~ProgrammationManager();
    ProgrammationManager(const ProgrammationManager& pgm) ; /*!< Privé pour empêcher la recopie */
    ProgrammationManager& operator=(const ProgrammationManager& pgm) ; /*!< Privé pour empêcher la recopie */
    struct Handler{
        ProgrammationManager* instance;
        Handler():instance(0){}
        // destructeur appelé à la fin du programme
        ~Handler(){ if (instance) delete instance; }
    }; /*!< Handler appliquant le design pattern Singleton */
    static Handler handler;
public :
    const Programmation *trouverEvenement(const QString& name); /*!< Retourne un pointeur sur la Programmation d'un Evenement, 0 si l'Evenement n'existe pas/ */
    bool plageHoraireDispo(const QDateTime &dt, const Duree& dur); /*!< True si aucune Programmation n'est en cours à une QDateTime précise pour la Durée indiquee */
    const QMap<QDateTime, Programmation*>& getProgrammations() const { return programmations; } /*!< Accesseur en lecture aux Programmation. */
    static ProgrammationManager& getInstance(); /*!< Retourne une référence sur l'objet ProgrammationManager, le créant si besoin. */
    static void libererInstance(); /*!< Détruit ProgrammationManager et réinitialise l'instance à 0 */
    QMap<QDateTime, Programmation *> getProgrammations(const TacheUnitaire* t) const; /*!< Retourne la liste des Programmation d'une TacheUnitaire. */
    Programmation& ajouterProgrammationTache(const TacheUnitaire* t, const QDateTime &dt, const Duree& dur); /*!< Si les conditions sont réunies, programme une TacheUnitaire à une date pour une certaine durée. */
    Programmation& ajouterProgrammationEvenement(const QString& n, const QString& t, const QString& d, const QString& l, const QDateTime &dt, const Duree& dur); /*!< Si les conditions sont réunies, programme et crée un Evenement à une date pour une certaine durée. */
    /*!
         *  \brief Suppression des programmations d'une TacheUnitaire
         *
         *  Methode qui permet de supprimer les Programmations d'une TacheUnitaire, dans le cadre de la suppression d'une TacheUnitaire, sans déprogrammer en cascade.
         *
         *  \param t : pointeur sur la TacheUnitaire.
         */
    void suppressionProgrammations(const TacheUnitaire* t);
    /*!
         *  \brief Déprogrammation d'une TacheComposite
         *
         *  Methode qui permet de déprogrammer une TacheComposite en déprogrammant toutes ses sous-tâches, et en déprogrammant aussi les Tache qui la suivent.
         *
         *  \param t : pointeur sur la TacheComposite.
         */
    void deprogrammationTotale(const TacheComposite* t);
    /*!
         *  \brief Déprogrammation d'une TacheUnitaire
         *
         *  Methode qui permet de déprogrammer une TacheUnitaire, en déprogrammant aussi les Tache qui la suivent.
         *
         *  \param t : pointeur sur la TacheUnitaire.
         */
    void deprogrammationTotale(const TacheUnitaire* t);
    /*!
         *  \brief Déprogrammation partielle d'une TacheUnitaire
         *
         *  Methode qui permet de déprogrammer une Programamtion particulière d'une TacheUnitaire,
         *  si elle existe, déprogramamnt totalement en cascade les Tache qui la suivent.
         *
         *  \param t : pointeur sur la TacheUnitaire.
         */
    void deprogrammationPartielle(const TacheUnitaire* t, const QDateTime &dt);
    /*!
         *  \brief Déprogrammations des Tache suivantes
         *
         *  Methode qui permet de déprogrammer totalement les Tache qui suivent.
         *
         *  \param t : pointeur sur la Tache (unitaire ou composite).
         */
    void deprogrammationSuiv(const Tache* t);
    /*!
         *  \brief Déprogramamtion d'un Evenement
         *
         *  Methode qui permet de détruire un Evenement et la Programmation à qui il est lié, s'il y a bien un EVenement à la QDateTime indiquée
         *
         *  \param dt : Date et Heure de l'Evenement.
         */
    void deprogrammationEvenement(const QDateTime& dt);
};
#endif

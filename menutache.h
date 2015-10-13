#ifndef EDITEURTACHE_H
#define EDITEURTACHE_H

#include <QDialog>
#include "Calendar.h"
#include <QStandardItemModel>
#include "message.h"
#include "ui_editeurtache.h"
#include "ui_ajoutprecedence.h"
#include "ui_nouvelleprecedence.h"
#include "ui_ajoutsoustache.h"
#include "ui_suppressiontache.h"
#include "ui_suppressionprecedence.h"
#include "ui_supprimersoustache.h"

/**
 * \file menutache.h
 * \brief Fenêtres utilisateur permettant de gérer les Tache et les relations qui les lient.
 */

/*///////////////////////////////////////////////////////*/
/*                     EditeurTache                     */
/*///////////////////////////////////////////////////////*/
/*! \class EditeurTache
        \brief Widget héritant de QDialog permettant d'ajouter une Tache, TacheUnitaire comme TacheComposite.
*/
class EditeurTache : public QDialog
{
    Q_OBJECT

public:
    explicit EditeurTache(QWidget *parent = 0);
    static QMap<QString, Tache*> predecesseurs;
    virtual ~EditeurTache();

private:
    Ui::EditeurTache *ui;

signals:
    void nouvelleTache();

public slots:
    void showPropUnit(bool b); /*!< Si le radio button "unitaire" est coché, affiche la partie propre aux TacheUnitaire. */
    void showPropComp(bool b); /*!< Si le radio button "composite" est coché, affiche la partie propre aux TacheComposite. */
    void setDates(QString s); /*!< Permet de mettre à jour les dates à la sélection d'un Projet, pour empêcher l'utilisateur de rentrer des dates incohérentes et le guider. */
    void remplirComboMere(QString s); /*!< Quand un Projet est sélectionné, propose ses TacheComposite comme tâche mère de la nouvelle Tache. */
    void choixPrec(); /*!< Permet de sélectionner des Tache précédentes à la création. */
    void enregisterTache(); /*!< Créer la Tache grâce au Projet sélectionné si les informations sont valides. */
};


/*///////////////////////////////////////////////////////*/
/*                     AjoutPrecedence                   */
/*///////////////////////////////////////////////////////*/

/*! \class AjoutPrecedence
        \brief Widget héritant de QDialog et permettant à l'utilisateur d'ajouter des liens de précédences à une Tache à sa création.
*/
class AjoutPrecedence : public QDialog
{
    Q_OBJECT

public:
    explicit AjoutPrecedence(QWidget *parent = 0);
    QList<QString> *testChecked(QStandardItem *element, QList<QString> *liste);
    ~AjoutPrecedence();
private:
    Ui::AjoutPrecedence *ui;
    int nbTachesAjoutees;
    QStandardItemModel modele; /*!< On utilise un modèle pour faire un QTreeView avec des checkbox pour sélectionner les Tache. */

public slots :
    void EnregistrerPred(); /*!< Retient la liste des Tache sélectionnées */
};


/*///////////////////////////////////////////////////////*/
/*                     Nouvelle Precedence               */
/*///////////////////////////////////////////////////////*/

/*! \class NouvellePrecedence
        \brief Widget héritant de QDialog et permettant à l'utilisateur d'ajouter des liens de précédences entre des Tache.
*/
class NouvellePrecedence : public QDialog
{
    Q_OBJECT

public:
    explicit NouvellePrecedence(QWidget *parent = 0);
    ~NouvellePrecedence();

private:
    Ui::NouvellePrecedence *ui;

public slots:
    void remplirComboTache1(QString); /*!< En fonction du Projet 1 choisi, remplit la QComboBox avec les Tache du Projet */
    void remplirComboTache2(QString); /*!< En fonction du Projet 2 choisi, remplit la QComboBox avec les Tache du Projet */
    void EnregistrerPrec(); /*!< La Tache 2 précèdera la Tache 1 si les conditions sont réunies. */
};

/*///////////////////////////////////////////////////////*/
/*                    Ajout Sous-Tâche                   */
/*///////////////////////////////////////////////////////*/

/*! \class AjoutSousTache
        \brief Widget héritant de QDialog et permettant à l'utilisateur d'ajouter des sous-tâches à une TacheComposite.
*/
class AjoutSousTache : public QDialog
{
    Q_OBJECT

public:
    explicit AjoutSousTache(QWidget *parent = 0);
    ~AjoutSousTache();

private:
    Ui::AjoutSousTache *ui;

public slots:
    void remplirComboTacheMere(const QString& p); /*!< En fonction du Projet choisi, remplit la QComboBox avec les TacheComposite du Projet */
    void remplirComboSousTache(const QString& p); /*!< En fonction du Projet choisi, remplit la QComboBox avec les Tache du Projet n'ayant pas de TacheComposite mère */
    void EnregistrerSousTache(); /*!< La relation de sous-tâche est créée si les condtitions sont réunies */
};

/*///////////////////////////////////////////////////////*/
/*                   Suppression Tâche                   */
/*///////////////////////////////////////////////////////*/

/*! \class SuppressionTache
        \brief Widget héritant de QDialog et permettant à l'utilisateur de supprimer une Tache.
*/
class SuppressionTache : public QDialog
{
    Q_OBJECT

public:
    explicit SuppressionTache(QWidget *parent = 0);
    ~SuppressionTache();

private:
    Ui::SuppressionTache *ui;

public slots:
    void remplirComboTache(const QString& p); /*!< En fonction du Projet choisi, remplit la QComboBox avec les Tache du Projet */
    void supprimerTache(); /*!< Permet la suppression de la Tache en assurant le maintien de la coéhrence des données */
};

/*///////////////////////////////////////////////////////*/
/*                Suppression Précédence                 */
/*///////////////////////////////////////////////////////*/

/*! \class SuppressionPrecedence
        \brief Widget héritant de QDialog et permettant à l'utilisateur de supprimer une précédence entre deux Tache.
*/
class SuppressionPrecedence : public QDialog
{
    Q_OBJECT

public:
    explicit SuppressionPrecedence(QWidget *parent = 0);
    ~SuppressionPrecedence();

private:
    Ui::SuppressionPrecedence *ui;

public slots:
    void remplirComboTache(const QString& p); /*!< En fonction du Projet choisi, remplit la QComboBox avec les Tache du Projet */
    void showPrecedence(const QString& t); /*!< En fonction de la Tache choisie, affiche les Tache la précédant et en permet la sélection */
    void supprimerPrec(); /*!< Supprime la précédence si els champs sont renseignés */
};

/*///////////////////////////////////////////////////////*/
/*                 Suppression Sous-Tâche                */
/*///////////////////////////////////////////////////////*/

/*! \class SupprimerSousTache
        \brief Widget héritant de QDialog et permettant à l'utilisateur de supprimer une sous-tâche d'une TacheComposite.
*/
class SupprimerSousTache : public QDialog
{
    Q_OBJECT

public:
    explicit SupprimerSousTache(QWidget *parent = 0);
    ~SupprimerSousTache();

private:
    Ui::SupprimerSousTache *ui;

public slots:
    void remplirComboTacheMere(const QString& p); /*!< En fonction du Projet choisi, remplit la QComboBox avec les TacheComposite du Projet */
    void remplirComboSousTache(const QString& t); /*!< En fonction de la TacheComposite choisie, remplit la QComboBox avec les sous-tâche de la TacheComposite */
    void suppressionSousTache(); /*!< Si les champs sont remplis, les deux Tache ne sont plus liées par une relation de sous-tâche */
};
#endif // EDITEURTACHE_H

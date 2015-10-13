#ifndef MENUEVENEMENT_H
#define MENUEVENEMENT_H

#include <QDialog>
#include <QStandardItemModel>
#include <QStandardItem>
#include "Calendar.h"
#include "message.h"
#include "ui_progactivite.h"
#include "ui_progtache.h"
#include "ui_deprogtache.h"
#include "ui_deprogactivite.h"

/**
 * \file menuevenement.h
 * \brief Fenêtres utilisateur permettant de gérer les Programmation, ajout comme suppression, TacheUnitaire comme Evenement.
 */

/**********************************************************************/
/*            Onglet Programmation d'une activité                     */
/**********************************************************************/

/*! \class ProgActivite
        \brief Widget héritant de QDialog et permettant de créer un Evenement lié à une Programmation.
*/
class ProgActivite : public QDialog
{
    Q_OBJECT

public:
    explicit ProgActivite(QWidget *parent = 0);
    ~ProgActivite();
private:
    Ui::ProgActivite *ui;
public slots :
    void EnregistrerProg(); /*!< Créer la Programmation avec l'Evenement si les informations sont valides. */
};


/**********************************************************************/
/*            Onglet Programmation d'une Tache                        */
/**********************************************************************/

/*! \class ProgTache
        \brief Widget héritant de QDialog et permettant de programmer des TacheUnitaire.
*/
class ProgTache : public QDialog
{
    Q_OBJECT

public:
    explicit ProgTache(QWidget *parent = 0);
    ~ProgTache();
private:
    Ui::ProgTache *ui;
public slots :
    void remplirComboTache(const QString &p); /*!< En fonction du Projet choisi, remplit la QCOmboBox de TacheUnitaire pas encore programmée et dont la date d'échéance n'est pas passée */
    void SetInfos(const QString& t); /*!< Quand une TacheUnitaire est sélectionnée, affiche les informations de bases et la durée restantes à programmer. */
    void EnregistrerProgTache(); /*!< Si la préemption est respectée, les dates cohérentes, la plage horaire disponible, créer la Programmation de la TacheUnitaire */
};

/**********************************************************************/
/*            Onglet Deprogrammation d'une Tache                      */
/**********************************************************************/

/*! \class DeprogTache
        \brief Widget héritant de QDialog et permettant de déprogrammer partiellement ou totalement des TacheUnitaire.
*/
class DeprogTache : public QDialog
{
    Q_OBJECT

public:
    explicit DeprogTache(QWidget *parent = 0);
    ~DeprogTache();
private:
    Ui::DeprogTache *ui;
public slots :
    void showPropPart(bool b); /*!< Si le radio button "partielle" est coché, affiche les différentes programmations de la TacheUnitaire */
    void showPropTot(bool b); /*!< Si le radio button "totale" est coché, cache les différentes programmations de la TacheUnitaire */
    void remplirComboTache(const QString& p); /*!< En fonction du Projet choisi, remplit la QCOmboBox de TacheUnitaire si la Programmation n'est pas passée */
    void showProgrammation(const QString& t); /*!< Permet de voir les différenets Programmation d'une TacheUnitaire et en sélectionner une à déprogrammer dans le cas de la déprogramamtion partielle */
    void EnregistrerDeprogTache(); /*!< Si les informations sont valides, déprogramme la TacheUnitaire, partiellement ou totalement */
};

/**********************************************************************/
/*            Onglet Deprogrammation d'une Activité                   */
/**********************************************************************/

/*! \class DeprogActivite
        \brief Widget héritant de QDialog et permettant de déprogrammer un Evenement.
*/
class DeprogActivite : public QDialog
{
    Q_OBJECT

public:
    explicit DeprogActivite(QWidget *parent = 0);
    ~DeprogActivite();
private:
    Ui::DeprogActivite *ui;
public slots :
    void remplirComboBoxActivite(); /*!< Montre la liste des Evenement */
    void showProgrammation(const QString& name); /*!< Montre les informations sur l'Evenement et sa Programmation */
    void EnregistrerDeprogActivite(); /*!< Si els champs sont correctes, détruit l'Evenement et sa Programamtion */
};


#endif // MENUEVENEMENT_H

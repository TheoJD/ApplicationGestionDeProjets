#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QStandardItemModel>
#include <QTreeWidgetItem>
#include <QFileDialog>
#include "Calendar.h"
#include "menuprojet.h"
#include "menutache.h"
#include "menuevenement.h"
#include "emploidutemps.h"
#include "ui_mainwindow.h"
//#include "ui_frontpage.h"
#include <QTreeView>

/**
 * \file mainwindow.h
 * \brief Fenêtre utilisateur permettant d'accéder à toutes les fonctionnalités du programme.
 */
/**********************************************************************/
/*                        MainWindow                                       */
/**********************************************************************/

/*! \class TreeWidgetItem
        \brief Widget héritant de QTreeWidgetItem et permettant l'ajout récursif des sous-tâches des TacheComposite afin d'obtenir une vraie arborescence.
*/
class TreeWidgetItem : public QTreeWidgetItem {
public :
    explicit TreeWidgetItem() : QTreeWidgetItem() {}
    ~TreeWidgetItem() {}
    void setSousTaches(const TacheComposite* t); /*!< Permet d'accrocher les sous-âches à leur TacheComposite mère. */
};

/*! \class MainWindow
        \brief Widget héritant de QMainWindow et permettant à l'utilisateur de voir une TreeView des Projet et Tache ainsi que d'accéder à l'ensemble des fonctionnalités de l'application.
*/
class MainWindow : public QMainWindow
{
    Q_OBJECT
    QString file; /*!< Pour conserver le nom du fichier chargé */
public:
    explicit MainWindow(QWidget *parent = 0);
    ~MainWindow();
    void setTree(QTreeWidget *arbre); /*!< Créer le TreeView initial */

public slots:
    void updateTree(); /*!< Met à jour l'arbre au sortir de chaque fenêtre utilisateur */
    void creerProjet(); /*!< Ouvre la fenêtre CreationProjet de création de Projet */
    void supprimerProjet(); /*!< Ouvre la fenêtre SuppressionProjet de suppression de Projet */
    void creerTache(); /*!< Ouvre la fenêtre AjoutTahce de création de Tache */
    void nouvellePrec(); /*!< Ouvre la fenêtre NouvellePrecedence d'e création de Projet d'ajout de précédences entre Tache */
    void ajouterSousTache(); /*!< Ouvre la fenêtre AjotuSousTache d'e création de Projet d'ajout de sous-tâche à une TacheComposite */
    void supprimerTache(); /*!< Ouvre la fenêtre SuppressionTache de suppression de Tache */
    void supprimerPrecedence(); /*!< Ouvre la fenêtre SuppressionPrecedence de suppression de précédences entre Tache */
    void suppressionSousTache(); /*!< Ouvre la fenêtre SupprimerSousTache de suppression de sous-tâche à une TacheComposite */
    void programAct(); /*!< Ouvre la fenêtre ProgActivite pour créer et programmer des Evenement */
    void programTache(); /*!< Ouvre la fenêtre ProgTache pour programmer des TacheUnitaire */
    void deprogrammerTache(); /*!< Ouvre la fenêtre DeprogTache pour déprogrammer totalement ou partiellement des TacheUnitaire */
    void deprogrammerActivite(); /*!< Ouvre la fenêtre DeprogActivite pour détuire des Evement et leur Programmation */
    void affichageDescription(QTreeWidgetItem * item, int column); /*!< Quand on sélectionne un QTreeWidgetItem, affiche els informations de la Tache ou du Projet associé à gauche de la fenêtre */
    void afficheEmploi(); /*!< Ouvre la fenêtre EmploiDuTemps pour visualiser un calendrier hebdomadaire */
    void sauverCalendrier(); /*!< Ouvre la fenêtre de dialogue pour sauvegarder un fichier .xml */
    void chargerCalendrier(); /*!< Ouvre la fenêtre de dialogue pour charger un fichier .xml */

private:
    Ui::MainWindow *ui;
};

/**********************************************************************/
/*                        FrontPage                                       */
/**********************************************************************/



/*class FrontPage : public QDialog
{
    Q_OBJECT

public:
    explicit FrontPage(QWidget *parent = 0);
    ~FrontPage();

private:
    Ui::FrontPage *ui;
public slots:
    void fenetrePrincipale();

};*/

#endif // MAINWINDOW_H

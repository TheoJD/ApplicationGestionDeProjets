#ifndef EMPLOIDUTEMPS_H
#define EMPLOIDUTEMPS_H

#include <QDialog>
#include <QStandardItemModel>
#include "Calendar.h"
#include "ui_emploidutemps.h"
#include "ui_semaine.h"

/**
 * \file emploidutemps.h
 * \brief Fenêtre utilisateur permettant la visualisation d'une semaine du calendrier avec les programmations et informations sur les Tache et Evenement prévus.
 */

/*! \class Semaine
        \brief Widget héritant de QDialog proposant à l'utilisateur de choisir une semaine du calendrier.
*/
class Semaine : public QDialog
{
    Q_OBJECT

public:
    explicit Semaine(QWidget *parent = 0);
    ~Semaine();

private:
    Ui::Semaine *ui;

public slots:
    void afficheEdt(QDate d); /*!< Ouvre l'emploi du temps avec la semaine choisie */
};

/*! \class EmploiDuTemps
        \brief Widget héritant de QDialog montrant à l'utilisateur le programme de ses Tache et Evenement sur une semaine.
*/
class EmploiDuTemps : public QDialog
{
    Q_OBJECT

public:
    explicit EmploiDuTemps(QDate d, QWidget *parent = 0);
    ~EmploiDuTemps();


private:
    Ui::EmploiDuTemps *ui;
    QDate lundi;
public slots :
    void changeEdt(); /*!< Permet de modifier la semaine affichée */
    void afficheInfos(int row, int column); /*!< Quand on sélectionne une Programmation de TacheUnitaire ou d'Evenement, affiche des informations complémentaires sur la TacheUnitaire ou l'Evenement. */
};

#endif // EMPLOIDUTEMPS_H

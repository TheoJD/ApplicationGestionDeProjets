#ifndef CREATIONPROJET_H
#define CREATIONPROJET_H

#include <QDialog>
#include "Calendar.h"
#include "mainwindow.h"
#include "ui_creationprojet.h"
#include "ui_suppressionprojet.h"


/**
 * \file menuprojet.h
 * \brief Fenêtres utilisateur permettant de gérer la création et la suppression de Projet.
 */

/**********************************************************************/
/*                  Onglet Creer nouveau projet                       */
/**********************************************************************/

/*! \class CreationProjet
        \brief Widget héritant de QDialog et permettant de créer des Projet.
*/
class CreationProjet : public QDialog
{
    Q_OBJECT

public:
    explicit CreationProjet(QWidget *parent = 0);
    virtual ~CreationProjet();

private:
    Ui::CreationProjet *ui;

public slots:
    void activerOK(); /*!< La possibilité de créer un Projet s'active une fois les champs renseignés */
    void save(); /*!< Créer le Projet si les informations sont correctes */

};

/**************************************************************************************************************************************/

/**********************************************************************/
/*                          Supprimmer projet                         */
/**********************************************************************/

/*! \class SuppressionProjet
        \brief Widget héritant de QDialog et permettant de supprimer un Projet.
*/
class SuppressionProjet : public QDialog
{
    Q_OBJECT

public:
    explicit SuppressionProjet(QWidget *parent = 0);
    virtual ~SuppressionProjet();

private:
    Ui::SuppressionProjet *ui;

public slots:
    void remplirComboBoxProjet(); /*!< Propose la liste des Projet. */
    void supprimmerProjet(); /*!< Si un projet est sélectionné, le supprime. */

};


#endif // CREATIONPROJET_H

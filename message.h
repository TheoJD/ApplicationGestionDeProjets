#ifndef MESSAGE_H
#define MESSAGE_H

#include <QMessageBox>

/**
 * \file message.h
 * \brief Contient les classes héritées de QMessageBox pour proposer à l'utilisateur de faire certains choix.
 */

/*! \class MessageValidation
        \brief Widget permettant de proposer à l'utilisateur de continuer ou non une activité
        comme l'ajout de tâche, la programmation, la suppression de précédence etc.
*/
class MessageValidation : public QMessageBox{
public:
    MessageValidation(const QString& titre, const QString& message) {
        setWindowTitle(titre);
        setText(message);
        setInformativeText(QString("Voulez-vous continuer ?"));
        QFont f = QFont();
        f.setItalic(true);
        setFont(f);

        setIcon(QMessageBox::Question);
        setStandardButtons(QMessageBox::Yes | QMessageBox::No);
    }
};

/*! \class MessageSauvegarde
        \brief Widget demandant à l'utilisateur au moment où il charge un calendrier s'il veut sauvegarde le précédent.
*/
class MessageSauvegarde : public QMessageBox{
public:
    MessageSauvegarde(const QString& titre, const QString& message) {
        setWindowTitle(titre);
        setText(message);
        setInformativeText(QString("Voulez-vous enregistrer le calendrier actuel ?"));
        QFont f = QFont();
        f.setItalic(true);
        setFont(f);

        setIcon(QMessageBox::Question);
        setStandardButtons(QMessageBox::Yes | QMessageBox::No);
    }
};

#endif // MESSAGE_H


#include "emploidutemps.h"
#include "ui_emploidutemps.h"
#include "ui_semaine.h"

/**
 * \file emploidutemps.cpp
 * \brief Défini les méthodes de emploidutemps.h
 */

/****************************************************/
/*           Fenetre demandant la semaine           */
/****************************************************/

Semaine::Semaine(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::Semaine)
{
    ui->setupUi(this);
    QObject::connect(ui->pushButtonAnnuler, SIGNAL(clicked()),this,SLOT(close()));
    QObject::connect(ui->calendarWidget, SIGNAL(activated(QDate)),this,SLOT(afficheEdt(QDate)));
}


void Semaine::afficheEdt(QDate d){

    QDate d2 = d.addDays(- (d.dayOfWeek()-1));
    this->close();
    EmploiDuTemps* edt= new EmploiDuTemps(d2);
    edt->show();

}

Semaine::~Semaine()
{
    delete ui;
}

/**************************************/
/*           EmploiDuTemps            */
/**************************************/

EmploiDuTemps::EmploiDuTemps(QDate d, QWidget *parent) :
    QDialog(parent), lundi(d),
    ui(new Ui::EmploiDuTemps)
{
    ui->setupUi(this);
    ui->stackedWidget->setHidden(true);

    ui->labelNumero->setText(QDate::shortDayName(d.dayOfWeek())+d.toString(Qt::SystemLocaleShortDate)+" au "+QDate::shortDayName(d.addDays(6).dayOfWeek())+d.addDays(6).toString(Qt::SystemLocaleShortDate));

    ui->tableWidget->setColumnCount(7);
    QStringList jours(QDate::shortDayName(d.dayOfWeek())+d.toString(Qt::SystemLocaleShortDate));
        for (unsigned int i=1; i<7; i++) {
            jours.append((QDate::shortDayName(d.addDays(i).dayOfWeek()))+(d.addDays(i)).toString(Qt::SystemLocaleShortDate));
        }
    ui->tableWidget->setHorizontalHeaderLabels(jours);
    ui->tableWidget->verticalHeader()->setHidden(true);

    unsigned int nbRow =0;
    ui->tableWidget->setRowCount(nbRow);



    QDate jour1(d.year(), d.month(), d.day());
    QDate jour7=d.addDays(7);
    unsigned int prog = 0;


    QBrush coulEvt (QColor(191,117,255));
    QBrush coulTache (QColor(85,255,127));

    for (QDate jour=jour1; jour<jour7; jour = jour.addDays(1)){
            unsigned int nb=0;
            for(QMap<QDateTime, Programmation*>::const_iterator it = ProgrammationManager::getInstance().getProgrammations().cbegin();
                it != ProgrammationManager::getInstance().getProgrammations().cend();
                it++){
                if(it.key().date()== jour){
                    //afficher
                    QTableWidgetItem* item = new QTableWidgetItem;
                    if(it.value()->getEvenement()!=0){
                        item->setWhatsThis(it.value()->getEvenement()->getNom());
                        item->setText((*it)->getTime().toString("HH:mm")+" : "+(*it)->getEvenement()->getNom()+"\n\ndurée : "+(*it)->getDuree().stringDuree()+"\n\n");
                        item->setBackground(coulEvt);
                        prog++;
                    }
                    else if (it.value()->getTache()!=0){
                        item->setWhatsThis(it.value()->getTache()->getId());
                        item->setText((*it)->getTime().toString("HH:mm")+" : "+(*it)->getTache()->getId()+"\n\ndurée : "+(*it)->getDuree().stringDuree()+" \n\n");
                        item->setBackground(coulTache);
                        prog++;
                    }
                    if(nb>=nbRow){
                        nbRow+=5;
                        ui->tableWidget->setRowCount(nbRow);
                    }
                    ui->tableWidget->setItem(nb,(jour.dayOfWeek()-1),item);

                    nb++;
                }
            }

        }


    ui->tableWidget->resizeRowsToContents();
    if(prog==0)
        QMessageBox::information(this, "Message", "Il n'y a pas encore de programmation pour la semaine choisie.");



    QObject::connect(ui->pushButtonFermer, SIGNAL(clicked()),this,SLOT(close()));
    QObject::connect(ui->pushButtonChange, SIGNAL(clicked()),this,SLOT(changeEdt()));
    QObject::connect(ui->tableWidget, SIGNAL(cellClicked(int,int)),this,SLOT(afficheInfos(int, int)));

}





/****************SLOTS*****************/
void EmploiDuTemps::changeEdt(){
    this->close();
    Semaine* s = new Semaine(this);
    s->show();
}

void EmploiDuTemps::afficheInfos(int row, int column){
    if(ui->tableWidget->item(row, column)){ //si la case contient une programmation
        try {
            ui->stackedWidget->setHidden(false);
            QTableWidgetItem* item = ui->tableWidget->item(row, column);
            if(item->background()== QBrush(QColor(191,117,255))){
                //l'item est un evt
                ui->stackedWidget->setCurrentIndex(0);
                const Programmation* evt = ProgrammationManager::getInstance().trouverEvenement(item->whatsThis());
                ui->labelDescr->setText(evt->getEvenement()->getDescription());
                ui->labelLieu->setText(evt->getEvenement()->getLieu());
                ui->labelNom->setText(evt->getEvenement()->getNom());
                ui->labelDate->setText(evt->getDate().toString(Qt::SystemLocaleShortDate));
                ui->labelDuree->setText(evt->getDuree().stringDuree());
                ui->labelType->setText(evt->getEvenement()->getType());
                ui->labelHor->setText(evt->getTime().toString("HH:mm"));

            }
            else if (item->background()==QBrush(QColor(85,255,127))){
                //l'item est une tache
                ui->stackedWidget->setCurrentIndex(1);
                Tache* t = ProjetManager::getInstance().trouverTache(item->whatsThis());
                ui->labelId->setText(t->getId());
                ui->labelTitre_2->setText(t->getTitre());
                ui->labelProjet->setText(t->getProjet()->getNom());
                ui->labelDate_2->setText(ui->tableWidget->horizontalHeaderItem(column)->text());
                ui->labelHor_2->setText(item->text().section(" ",0,0));
                ui->labelDuree_2->setText("\n\n"+item->text().section("durée : ",1,1));

            }
        }catch(CalendarException e){
            QMessageBox::warning(this, "Erreur", e.getInfo());
        }
    }
    else //si la case est vide
        ui->stackedWidget->setHidden(true);
}

/***************************************/

EmploiDuTemps::~EmploiDuTemps()
{
    delete ui;
}

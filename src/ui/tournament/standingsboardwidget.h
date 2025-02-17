#pragma once
#include <QDialog>
#include "../../model/abstract_tournament.h"
#include "../../model/round.h"
#include "../../model/player.h"
#include "../widgets/searchsorttablewidget.h"
#include "../abstractmodels/playerscoremodel.h"

namespace Ui
{
class StandingsBoardWidget;
}

class StandingsBoardWidget : public QDialog
{
    Q_OBJECT

public:
    explicit StandingsBoardWidget(Tournament *tourn, QWidget *parent = nullptr);
    ~StandingsBoardWidget();
public slots:
    void roundsChanged(std::vector<Round>);
    void playersChanged(std::vector<Player>);
protected:
    void changeEvent(QEvent *e);
private:
    void redrawStandingsBoard();

    QVBoxLayout *tableLayout;
    SearchSortTableWidget<PlayerScoreModel, PlayerScore> *table;
    Ui::StandingsBoardWidget *ui;
    Tournament *tourn;
};


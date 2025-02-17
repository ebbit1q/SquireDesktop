#include "./roundviewwidget.h"
#include "./ui_roundviewwidget.h"
#include <QMessageBox>

RoundViewWidget::RoundViewWidget(Tournament *tourn, QWidget *parent) :
    QWidget(parent),
    ui(new Ui::RoundViewWidget)
{
    ui->setupUi(this);
    this->tourn = tourn;
    this->roundSelected = false;

    // Init the tables
    this->playerTableLayout = new QVBoxLayout(ui->playerInRoundTable);
    this->playerTableLayout->setAlignment(Qt::AlignTop);

    std::vector<Player> players;
    this->playerTable = new SearchSortTableWidget<PlayerModel, Player>(players);
    this->playerTableLayout->addWidget(playerTable);

    // Init the results
    this->resultsLayout = new QVBoxLayout(ui->resultEntryWidget);
    this->resultsLayout->setAlignment(Qt::AlignTop);

    connect(this->tourn, &Tournament::onPlayersChanged, this, &RoundViewWidget::onPlayersChanged);
    connect(&this->timeLeftUpdater, &QTimer::timeout, this, &RoundViewWidget::displayTime);
    connect(this->playerTable->selectionModel(), &QItemSelectionModel::selectionChanged, this, &RoundViewWidget::onPlayerSelected);
    this->results = new RoundResults();
    this->displayRound();

    connect(ui->saveResults, &QPushButton::clicked, this, &RoundViewWidget::onResultsSave);
    connect(ui->resetResults, &QPushButton::clicked, this, &RoundViewWidget::displayRound);
    connect(ui->confirmMatch, &QPushButton::clicked, this, &RoundViewWidget::confirmMatch);
    connect(ui->killMatch, &QPushButton::clicked, this, &RoundViewWidget::confirmKill);
}

RoundViewWidget::~RoundViewWidget()
{
    for (RoundResultWidget *w : this->resultWidgets) {
        this->resultsLayout->removeWidget(w);
        delete w;
    }
    delete results;

    delete playerTable;
    delete playerTableLayout;
    delete ui;
}

void RoundViewWidget::changeEvent(QEvent *e)
{
    QWidget::changeEvent(e);
    switch (e->type()) {
    case QEvent::LanguageChange:
        ui->retranslateUi(this);
        break;
    default:
        break;
    }
}

QString RoundViewWidget::matchNumberToStr(int number)
{
    QString base = tr("Match #");
    if (number < 10) {
        base += tr("0");
    }

    base += QString::number(number, 10);

    return base;
}

void RoundViewWidget::rerender()
{
    this->displayRound();
}

void RoundViewWidget::displayRound()
{
    // Genereate state strings
    QString statusStr = tr("No Match Selected");
    QString numberStr = tr("Match #--");

    if (this->roundSelected) {
        this->timeLeftUpdater.start(100);
        this->playerTable->setData(this->round.players());
        numberStr = matchNumberToStr(this->round.match_number());

        // Status
        switch(this->round.status()) {
        case squire_core::sc_RoundStatus::Open:
            statusStr = tr("Match is in progress");
            break;
        case squire_core::sc_RoundStatus::Uncertified:
            statusStr = tr("Match is waiting results certification");
            break;
        case squire_core::sc_RoundStatus::Certified:
            statusStr = tr("Match has been finished and, results are confirmed");
            break;
        case squire_core::sc_RoundStatus::Dead:
            statusStr = tr("Match has been deleted");
            break;
        }
    } else {
        this->playerTable->setData(std::vector<Player>());
    }

    this->displayTime();

    // Render values in GUI
    ui->matchNumber->setText(numberStr);
    ui->roundStatus->setText(statusStr);

    // Results
    for (RoundResultWidget *w : this->resultWidgets) {
        this->resultsLayout->removeWidget(w);
        delete w;
    }
    delete this->results;

    this->resultWidgets = std::vector<RoundResultWidget *>();
    this->results = new RoundResults(this->round);

    for (Player p : this->round.players()) {
        RoundResultWidget *w = new RoundResultWidget(this->results, p, this);
        this->resultWidgets.push_back(w);
        this->resultsLayout->addWidget(w);
    }

    ui->drawsEdit->setValue(this->results->draws());
}

void RoundViewWidget::displayTime()
{
    int timeLeft = 0;
    int duration = 0;
    if (this->roundSelected) {
        timeLeft = this->round.time_left();
        duration = this->round.duration();
    }

    if (duration == 0) {
        ui->timeLeftProgressBar->setValue(0);
    } else {
        ui->timeLeftProgressBar->setValue((100 * timeLeft) / duration);
    }

    // Timer
    QString extentionStr = tr("Time Extensions") + " ";
    QString timeLeftStr = "";
    if (timeLeft == 0) {
        timeLeftStr = tr("Match has ended");
    } else {
        int baseDuration = this->tourn->round_length();
        int extention = this->round.duration() - baseDuration;
        if (extention != 0) {
            extentionStr += "(";
            if (extention > 0) {
                extentionStr += tr("+");
            }
            extentionStr += QString::number(extention / 60) + " " + tr("Minutes Extension") + ")";
        }

        int seconds = timeLeft % 60;
        int minutes = ((timeLeft / 60) % 60);
        int hours = timeLeft / (60 * 60);

        if (hours < 10) {
            timeLeftStr += "0";
        }
        timeLeftStr += QString::number(hours);
        timeLeftStr += ":";

        if (minutes < 10) {
            timeLeftStr += "0";
        }
        timeLeftStr += QString::number(minutes);
        timeLeftStr += ":";

        if (seconds < 10) {
            timeLeftStr += "0";
        }
        timeLeftStr += QString::number(seconds);
        timeLeftStr += " " + tr("Left in Match");
    }

    ui->timeLeftLabel->setText(timeLeftStr);
    ui->timeExtensionsLabel->setText(extentionStr);
}

void RoundViewWidget::onPlayersChanged(std::vector<Player>)
{
    this->displayRound();
}

void RoundViewWidget::setRound(Round round)
{
    this->round = round;
    this->roundSelected = true;
    this->displayRound();
}

void RoundViewWidget::onPlayerSelected(const QItemSelection &selected, const QItemSelection deselected)
{
    QModelIndexList indexes = selected.indexes();
    if (indexes.size() > 0) {
        int index = indexes[0].row();
        Player plyr = this->playerTable->getDataAt(index);
        emit this->playerSelected(plyr);
    }
}

void RoundViewWidget::onResultsSave()
{
    if (!this->roundSelected) {
        lprintf(LOG_ERROR, "No round selected\n");
        return;
    }

    int draws = ui->drawsEdit->value();
    std::vector<Player> players;
    std::vector<int> wins;
    std::vector<bool> confirms;

    for (size_t i = 0; i < this->resultWidgets.size(); i++) {
        RoundResultWidget *w = this->resultWidgets[i];
        bool confirmed = w->confirmed();
        int winc = w->newWins();

        players.push_back(w->player());
        wins.push_back(winc);
        confirms.push_back(confirmed);
    }

    for (size_t i = 0; i < players.size() && i < wins.size() && i < confirms.size(); i++) {
        bool s = this->tourn->recordResult(this->round, players[i], wins[i]);
        if (confirms[i] && s) {
            s = this->tourn->confirmPlayer(this->round, players[i]);
        }

        if (!s) {
            QMessageBox msg;
            msg.setWindowTitle(tr("Cannot save results for - ")
                               + QString::fromStdString(players[i].all_names()));
            msg.setText(tr("Aborting: Cannot save results for - ")
                        + QString::fromStdString(players[i].all_names()));
            msg.exec();
            return;
        }
    }

    bool s = this->tourn->recordDraws(this->round, draws);

    if (!s) {
        QMessageBox msg;
        msg.setWindowTitle(tr("Cannot save draws"));
        msg.setText(tr("Cannot save draws"));
        msg.exec();
    }
}

void RoundViewWidget::confirmMatch()
{
    for (Player p : this->round.players()) {
        bool r = this->tourn->confirmPlayer(this->round, p);
        if (!r) {
            lprintf(LOG_ERROR, "Cannot confirm results for %s\n", p.all_names());

            QMessageBox msg;
            msg.setWindowTitle(tr("Cannot confirm all results in match."));
            msg.setText(tr("Cannot confirm all result for player ") + QString::fromStdString(p.all_names()));
            msg.exec();

            return;
        }
    }
}

void RoundViewWidget::confirmKill()
{
    QMessageBox msg;
    msg.setWindowTitle(tr("Are you sure you want to kill round ") + QString::number(this->round.match_number()));
    msg.setText(tr("Are you sure you want to kill round ") + QString::number(this->round.match_number()));
    msg.addButton(QMessageBox::Ok);
    msg.addButton(QMessageBox::Cancel);

    connect(&msg, &QMessageBox::accepted, this, &RoundViewWidget::kill);
    msg.exec();
}

void RoundViewWidget::kill()
{
    bool r = this->tourn->killRound(this->round);
    if (!r) {
        QMessageBox msg;
        msg.setWindowTitle(tr("Cannot kill round ") + QString::number(this->round.match_number()));
        msg.setText(tr("Cannot kill round ") + QString::number(this->round.match_number()));
        msg.exec();
    }
}


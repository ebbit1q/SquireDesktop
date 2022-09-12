#include "./roundviewwidget.h"
#include "./ui_roundviewwidget.h"

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

    connect(this->tourn, &Tournament::onPlayersChanged, this, &RoundViewWidget::onPlayersChanged);
    connect(&this->timeLeftUpdater, &QTimer::timeout, this, &RoundViewWidget::displayRound);
    connect(this->playerTable->selectionModel(), &QItemSelectionModel::selectionChanged, this, &RoundViewWidget::onPlayerSelected);
    this->displayRound();
}

RoundViewWidget::~RoundViewWidget()
{
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

void RoundViewWidget::displayRound()
{
    // Genereate state strings
    QString statusStr = tr("No Match Selected");
    QString numberStr = tr("Match #--");
    int timeLeft = 0;
    int duration = 0;

    if (this->roundSelected) {
        this->timeLeftUpdater.start(100);
        this->playerTable->setData(this->round.players());
        numberStr = matchNumberToStr(this->round.match_number());
        timeLeft = this->round.time_left();
        duration = this->round.duration();

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

    // Timer
    QString extentionStr = tr("Time Extensions") + " ";
    QString timeLeftStr = "";
    if (timeLeft == 0) {
        timeLeftStr = tr("Match has ended");
    } else {
        int baseDuration = this->tourn->round_length();
        int extention = timeLeft - baseDuration;
        if (extention != 0) {
            extentionStr += "(";
            if (extention > 0) {
                extentionStr += tr("+");
            }
            extentionStr += QString::number(extention) + " " + tr("Minutes Extension") + ")";
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

    // Render values in GUI
    ui->matchNumber->setText(numberStr);
    ui->roundStatus->setText(statusStr);
    ui->timeLeftLabel->setText(timeLeftStr);
    ui->timeExtensionsLabel->setText(extentionStr);

    if (duration == 0) {
        ui->timeLeftProgressBar->setValue(0);
    } else {
        ui->timeLeftProgressBar->setValue((100 * timeLeft) / duration);
    }
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


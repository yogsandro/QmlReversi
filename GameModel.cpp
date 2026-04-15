/*
 * GameModel.cpp
 *
 *  Created on: Apr 13, 2026
 *      Author: sandro
 */

#include "GameModel.h"

GameModel::GameModel(QObject *parent) :
	QObject(parent) {

	m_pGameRegistry = std::make_shared<GameRegistry>();
	m_pGame = std::make_shared<Game>(*m_pGameRegistry);

	m_pStrategy = std::make_shared<AlphaBetaEvaluationStrategy>(*m_pGameRegistry);
	m_pPlayer1 = std::make_shared<HumanPlayer>("Sandro");
	m_pPlayer2 = std::make_shared<S3Player>("S3\u03B1\u03B2-7", "\u03B1\u03B2 pruning (Depth=5)", 7, *m_pStrategy);

	m_humanHasPlayed = false;
	m_game.clear();
	m_state = false;

	m_computerPlayer = std::thread([this](){
		do {
			std::cout << "\nComputer is waiting for a game to start..." << std::endl;
			m_game.wait(false);
			std::cout << "\nComputer is ready." << std::endl;
			do {
				{
					std::cout << "\nComputer is waiting..." << std::endl;
					bool color = m_pGameRegistry->color(*m_pPlayer2);
					std::unique_lock<std::mutex> lock(m_mtx);
					m_cv_turn.wait(lock,[this, &color]{ return m_pGameRegistry->blackToPlay() == color; });
					if (!m_pGame->hasEnded()) {
						std::cout << "\nComputer move..." << std::endl;
						US move = m_pGameRegistry->player().play();
						setComputerMove(move);
						setInfo(move == 0 ?
								QString("Je passe...") :
								QString::asprintf("Je joue en %s", Util::string(move).c_str()));
						m_pGameRegistry->addMove(move);
						m_pGameRegistry->turnOver();
						emit computerMoveChanged();
						emit scoreChanged();
						emit requestPaint();
						if (m_pGame->hasEnded()) {
							gameEnd();
							break;
						}
					} else {
						break;
					}
					m_cv_turn.notify_one();
				}
			} while(true);
			m_cv_turn.notify_one();
		} while(true);

		std::cout << "\nLeaving computer's thread." << std::endl;
	});

	m_humanPlayer = std::thread([this](){
		do {
			std::cout << "\nPlayer's thread is waiting for a game to start..." << std::endl;
			m_game.wait(false);
			std::cout << "\nPlayer's thread is ready." << std::endl;
			do {
				{
					std::cout << "\nPlayer thread is waiting..." << std::endl;
					bool color = m_pGameRegistry->color(*m_pPlayer1);
					std::unique_lock<std::mutex> lock(m_mtx);
					m_cv_turn.wait(lock, [this, &color]{ return m_pGameRegistry->blackToPlay() == color;});
					if (!m_pGame->hasEnded()) {
						std::cout << "\nPlayer's move..." << std::endl;
						OthelloBoard& m_board = m_pGameRegistry->board();
						US move = 0;
						if (!m_board.hasMove(color)) {
							std::cout << "Vous devez passer !\n";
							setInfo(QString("Vous devez passer!"));
							m_pGameRegistry->addMove(move);
							m_pGameRegistry->turnOver();
							emit scoreChanged();
							emit requestPaint();
						} else {
							m_cv_player.wait(lock, [this]{return m_humanHasPlayed;});
							std::cout << "\nHuman player has played." << std::endl;
							m_humanHasPlayed = false;
						}
						if (m_pGame->hasEnded()) {
							gameEnd();
							break;
						}
						m_cv_turn.notify_one();
					} else {
						break;
					}
				}
			} while(true);
			m_cv_turn.notify_one();
		} while(true);
		std::cout << "\nLeaving player's thread." << std::endl;
	});
	std::cout << "\nAAA" << std::endl;
};

void GameModel::doStart() {
	std::cout << "\nStarting game..." << std::endl;
	setInfo(QString("A vous de jouer"));
	m_humanHasPlayed = false;
	m_pGame->init();
	setPlayerDesc(QString::asprintf("%s (%s) ", m_pGameRegistry->color(*m_pPlayer1) ? "Noirs" : "Blancs", m_pPlayer1->name().c_str()));
	setComputerDesc(QString::asprintf("%s (%s) ", m_pGameRegistry->color(*m_pPlayer2) ? "Noirs" : "Blancs", m_pPlayer2->name().c_str()));
	m_game.test_and_set();
	m_game.notify_all();
	trigger();
}

void GameModel::gameEnd() {
	auto b1 = m_pGameRegistry->countPieces(*m_pPlayer1);
	auto b2 = m_pGameRegistry->countPieces(*m_pPlayer2);
	if (b1 == b2) {
		setInfo("Match nul");
	} else {
		setInfo(QString::asprintf("%s a gagné!!", b1 > b2 ? m_pPlayer1->name().c_str() : m_pPlayer2->name().c_str()));
	}
	m_pGame->end();
	m_game.clear();
	m_state = false;
}

Q_INVOKABLE	QVariantMap GameModel::getBoard() {
	QVariantMap map;
	Board& blackBoard = m_pGameRegistry->board().blackBoard();
	Board& whiteBoard = m_pGameRegistry->board().whiteBoard();
	QVariantList black;
	QVariantList white;
	for (unsigned short i {0}; i < 8; ++i) {
		for (unsigned short j {0}; j < 8; ++j) {
			if (blackBoard.is(j+1, i+1)) {
				QVariantMap m; m["x"] = j+1; m["y"] = i+1;
				black.append(m);
			} else if (whiteBoard.is(j+1, i+1)) {
				QVariantMap m; m["x"] = j+1; m["y"] = i+1;
				white.append(m);
			}
		}
	}
	map["black"] = black;
	map["white"] = white;
	return map;
}

Q_INVOKABLE void GameModel::trigger() { emit requestPaint(); }

Q_INVOKABLE bool GameModel::play(US move) {
	bool hasPlayed = false;
	OthelloBoard& board = m_pGameRegistry->board();
	if (!m_state) {
		if (board.board(true).is(move)) {
			std::cout << "\nBlack chosen..." << std::endl;
			m_pGameRegistry->addPlayer(*m_pPlayer1);
			m_pGameRegistry->addPlayer(*m_pPlayer2);
			m_state = true;
			doStart();
		} else if (board.board(false).is(move)){
			std::cout << "\nWhite chosen..." << std::endl;
			m_pGameRegistry->addPlayer(*m_pPlayer2);
			m_pGameRegistry->addPlayer(*m_pPlayer1);
			m_state = true;
			doStart();
		}
	} else {
		std::cout << "\nPlayer move..." << std::endl;
		{
			std::unique_lock<std::mutex> lock(m_mtx);
			bool blackToPlay = m_pGameRegistry->blackToPlay();
			if (board.canPlay(move, blackToPlay)) {
				board.play(move, blackToPlay);
				hasPlayed = true;
				setInfo(QString::asprintf("Vous jouez en %s", Util::string(move).c_str()));
				m_pGameRegistry->addMove(move);
				m_pGameRegistry->turnOver();
				setPlayerMove(move);
				emit playerMoveChanged();
				emit scoreChanged();
				emit requestPaint();
				m_humanHasPlayed = true;
				//notify human player's thread that human player has played
				m_cv_player.notify_one();
			}
		}
	}
	return hasPlayed;
}

Q_INVOKABLE	void GameModel::start() {
	if (!m_state) {
		setInfo(QString("Choisissez une couleur en cliquant sur une des pièces"));
	}
//		else {
//			if (m_pGame->hasStarted()) {
//				if (m_pGame->hasEnded()) {
//					doStart();
//				}
//			} else {
//				doStart();
//			}
//		}
}

Q_INVOKABLE	void GameModel::play() {
	if (!m_pGame->hasEnded()) m_pGame->play(true);
	else {
		m_pGame->end();
	}
}

void GameModel::setPlayerMove(US move) {
	bool black = true;
	OthelloBoard& board = m_pGameRegistry->board();
	m_PlayerMove = QString::asprintf("%2s %2d pions (%2d/%2d/%2d)", Util::string(move).c_str(),
			board.board(black).count(), board.border(black), board.mobility(black),
			board.corner(black));
}

void GameModel::setComputerMove(US move) {
	bool black = false;
	OthelloBoard& board = m_pGameRegistry->board();
	m_ComputerMove = QString::asprintf("%2s  %2d pions (%2d/%2d/%2d)", Util::string(move).c_str(),
			board.board(black).count(), board.border(black), board.mobility(black),
			board.corner(black));
}

QString GameModel::getComputerMove() const {
	return m_ComputerMove;
}

QString GameModel::getPlayerMove() const {
	return m_PlayerMove;
}

void GameModel::setInfo(const QString& s) {
	m_info = std::move(s);
	emit infoChanged();
}

QString GameModel::getInfo() const {
	return m_info;
}

const QString& GameModel::getComputerDesc() const {
	return m_computerDesc;
}

const QString& GameModel::getPlayerDesc() const {
	return m_playerDesc;
}

void GameModel::setComputerDesc(const QString &mComputerDesc) {
	m_computerDesc = mComputerDesc;
	computerDescChanged();
}

void GameModel::setPlayerDesc(const QString &mPlayerDesc) {
	m_playerDesc = mPlayerDesc;
	playerDescChanged();
}

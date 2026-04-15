/*
 * GameModel.h
 *
 *  Created on: Apr 13, 2026
 *      Author: sandro
 */

#pragma once

#include <thread>
#include <condition_variable>
#include <mutex>

#include <QVariant>

#include "othello/player/HumanPlayer.h"
#include "othello/player/S3Player.h"
#include "othello/GameRegistry.h"
#include "othello/Game.h"
#include "othello/OthelloBoard.h"


using namespace othello;

class GameModel : public QObject {
	Q_OBJECT
	Q_PROPERTY(QString computerMove READ getComputerMove NOTIFY computerMoveChanged)
	Q_PROPERTY(QString playerMove READ getPlayerMove NOTIFY playerMoveChanged)
	Q_PROPERTY(QString info READ getInfo NOTIFY infoChanged)
	Q_PROPERTY(QString computerDesc READ getComputerDesc NOTIFY computerDescChanged)
	Q_PROPERTY(QString playerDesc READ getPlayerDesc NOTIFY playerDescChanged)
public:
	explicit GameModel(QObject *parent = nullptr);
	void doStart();
	void gameEnd();
	void setPlayerMove(US move);
	void setComputerMove(US move);
	QString getComputerMove() const;
	QString getPlayerMove() const;
	void setInfo(const QString& s);
	QString getInfo() const;
	const QString& getComputerDesc() const;
	const QString& getPlayerDesc() const;
	void setComputerDesc(const QString &mComputerDesc);
	void setPlayerDesc(const QString &mPlayerDesc);

	Q_INVOKABLE	QVariantMap getBoard();
	Q_INVOKABLE void trigger();
	Q_INVOKABLE bool play(US move);
	Q_INVOKABLE	void start();
	Q_INVOKABLE	void play();

signals:
	void scoreChanged();
    void requestPaint();
    void computerMoveChanged();
    void playerMoveChanged();
    void infoChanged();
    void playerDescChanged();
    void computerDescChanged();


private:
	std::shared_ptr<GameRegistry> m_pGameRegistry;
	std::shared_ptr<Game> m_pGame;
	std::shared_ptr<Player> m_pPlayer1;
	std::shared_ptr<Player> m_pPlayer2;
	std::shared_ptr<AlphaBetaEvaluationStrategy> m_pStrategy;
	std::thread m_computerPlayer;
	std::thread m_humanPlayer;
	bool m_humanHasPlayed;
	std::condition_variable m_cv_turn;
	std::condition_variable m_cv_player;
	std::atomic_flag m_game;
	mutable std::mutex m_mtx;
	QString m_ComputerMove;
	QString m_PlayerMove;
	QString m_info;
	QString m_playerDesc;
	QString m_computerDesc;
	bool m_state;
};

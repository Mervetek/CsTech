#pragma once
#include <QMainWindow>
#include "ui_MainWindow.h"

#include <QFileDialog>
#include <QMessageBox>
#include <QTextStream>
#include <QPalette>

class MainWindow : public QMainWindow
{
	Q_OBJECT

public:
	MainWindow(QWidget *parent = nullptr);
	~MainWindow();

private:
	Ui::MainWindow ui;
	std::vector<QString> allWords;
	QString selectedFileName;
	bool doesWordExist(std::vector<QString> words, QString word);
	std::vector<QString> lines;
	std::vector<QString> updatedLines;
	void writeFile(std::vector<QString> newLines);
	bool checkCharacter(QString chr, QString word);
	QString parseString(QString complexString, QString chr);
	QString parseIncomingString(QString incomingStr);
	private slots:
	void slot_OpenFileDialogAndReadSelectedFile();
	void slot_FindWord();
	void slot_ReplaceWord();
    void slot_DeleteWord();
};

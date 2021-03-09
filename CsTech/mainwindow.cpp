#include "mainwindow.h"
#include "ui_mainwindow.h"
#include <iostream>

MainWindow::MainWindow(QWidget *parent)
	: QMainWindow(parent)
{
	ui.setupUi(this);
	connect(ui.pbSelectFile, SIGNAL(clicked()), this, SLOT(slot_OpenFileDialogAndReadSelectedFile()));
	connect(ui.pbFind, SIGNAL(clicked()), this, SLOT(slot_FindWord()));
	connect(ui.pbReplace, SIGNAL(clicked()), this, SLOT(slot_ReplaceWord()));
    connect(ui.pbDelete, SIGNAL(clicked()), this, SLOT(slot_DeleteWord()));
	selectedFileName = "";
	ui.leCount->setText("0");
}

MainWindow::~MainWindow()
{
}

bool MainWindow::doesWordExist(std::vector<QString> words, QString word)
{
    //jenerik bir metoddur, girilen kelime okunan dosyada var ise true, yok ise false doner.
	bool result = false;
	for (size_t index = 0; index < words.size(); index++)
	{
		if (words[index] == word)
		{
			result = true;
			break;
		}
		else
			result = false;
	}
	return result;
}

bool MainWindow::checkCharacter(QString chr, QString word)
{
    //recursive aramalar icin yazilmis bir metoddur. Kullanıcının verdiği char a gore kelimeyi kontrol eder.
	if (word.at(0) == chr && word.at(word.size() - 1) == chr)
	{
		return true;
	}
	else
		return false;
}

QString MainWindow::parseString(QString complexString, QString chr)
{
    //kelimeden - veya * karakterlerini çıkarıp bir alt string elde eder.
	QString substring;
	if (checkCharacter(chr, complexString))
	{
		for (size_t index = 1; index < complexString.size() - 1; index++)
		{
			substring.append(complexString.at(index));
		}
	}
	return substring;
}

QString MainWindow::parseIncomingString(QString incomingStr)
{
    //Verilen stringden ilk ve son karakteri cikarip kalan degerleri string olarak doner.
	QString substring;
	for (size_t index = 1; index < incomingStr.size() - 1; index++)
	{
		substring.append(incomingStr.at(index));
	}
	return substring;
}

void MainWindow::writeFile(std::vector<QString> newLines)
{
	QFile file(selectedFileName);
	if (file.open(QFile::WriteOnly | QFile::Truncate))
	{
		QTextStream stream(&file);
		for (size_t index = 0; index < newLines.size(); index++)
		{
			stream << newLines[index] << '\n';
        }
		file.close();
	}
}

void MainWindow::slot_OpenFileDialogAndReadSelectedFile()
{
	allWords.clear();


    //Dosya adini elle girmek maliyetli olacagindan kullanıcıya filedialog uzerinden secim yaptırılır.
	QString filePath = QFileDialog::getOpenFileName(this,
		tr("Open Selected File"), "/home", tr("Txt Files (*.txt)"));

	selectedFileName = filePath;
	ui.cbFiles->addItem(selectedFileName);

	QFile file(filePath);

    if (!file.open(QIODevice::ReadOnly | QIODevice::Text)) {
		QMessageBox::information(0, "error", file.errorString());
		return;
	}

	QTextStream in(&file);

    //dosya satir satir okunur, cumle ve kelimelere ayrilip veri yapilarinda tutulur.
	while (!in.atEnd())
	{
		QString line = in.readLine();
		lines.push_back(line);
		QStringList fields = line.split(" ");
		for (size_t index = 0; index < fields.size(); index++)
		{
			allWords.push_back(fields[index]);
		}
	}
	file.close();
}

void MainWindow::slot_FindWord()
{
	ui.listWidget->clear();
	int count = 0;
	std::vector<QString> findingList;
	QString findingWord = ui.leWord->text();

	if (findingWord.size() == 0)
	{
		QMessageBox msg;
		msg.setText("Finding word can not be empty.");
		msg.exec();
		return;
	}

    //chr == - , -ada- (radar)
	if (checkCharacter("-", findingWord))
	{
		for (size_t index = 0; index < allWords.size(); index++)
		{
			if (allWords[index].size() == findingWord.size())
			{
				QString substring = parseString(findingWord, "-");
				QString incomingStr = parseIncomingString(allWords[index]);
				if (substring == incomingStr)
				{
					findingList.push_back(allWords[index]);
				}
			}
		}
	}
    // chr == * -> *ata* (salatalik, atamak)
	else if (checkCharacter("*", findingWord))
	{
		for (size_t index = 0; index < allWords.size(); index++)
		{
			QString substring = parseString(findingWord, "*");
			if (allWords[index].contains(substring))
			{
				findingList.push_back(allWords[index]);
			}
		}
	}
	else
	{
		for (size_t index = 0; index < allWords.size(); index++)
		{
			if (allWords[index] == findingWord)
			{
				count++;
				findingList.push_back(findingWord);
			}
			else if (allWords[index].contains(findingWord))
			{
				count++;
				findingList.push_back(allWords[index]);
			}
		}

	}

    //bulunan tum kelimeler
	for (size_t index = 0; index < findingList.size(); index++)
	{
		ui.listWidget->addItem(findingList[index]);
	}

    //aranan kelime dokumanda kac tane geciyorsa ekrana yazan kod parcasi
	ui.leCount->setText(QString::number(findingList.size()));
}

void MainWindow::slot_ReplaceWord()
{
	QPalette palette = ui.leOldWord->palette();
	palette.setColor(QPalette::Base, QColor(255, 255, 255));
	ui.leOldWord->setPalette(palette);
	updatedLines.clear();
	QString oldWord = ui.leOldWord->text();
	QString newWord = ui.leNewWord->text();

    //Replace edilecek kelime bos ise kullaniciya hata mesaji doner.
	if (oldWord.size() == 0)
	{
		QMessageBox msg;
		QPalette palette = ui.leOldWord->palette();
		palette.setColor(QPalette::Base, QColor(255, 0, 0)); //Red
		ui.leOldWord->setPalette(palette);
		msg.setText("This field can not be empty.");
	}

    //aranan kelime bulunamadiysa kullanciya hata mesaji doner ve metoddan cikilir.
	if (!doesWordExist(allWords, oldWord))
	{
		QMessageBox msg;
		msg.setText(oldWord + " can not find selected file.");
		msg.exec();
        return;
	}

    //hata durumlarina dusmeyen durumda asagisi isletilir.
    //replace edilecek kelime tüm kelimelerde aranir ve degistirilir.
    //tum islemler yapildiktan sonra ayni dosyaya yazdirilir.
	else
	{
		for (size_t index = 0; index < lines.size(); index++)
		{
			QString updatedLine;
			QString line = lines[index];
			auto words = line.split(" ", QString::SkipEmptyParts);
			for (size_t j = 0; j < words.size(); j++)
			{
				if (words[j] != oldWord)
				{
					updatedLine.append(words[j]);
				}
				else
				{
					updatedLine.append(newWord);
				}
                if (j != words.size() -1)
                {
                    updatedLine.append(" ");
                }
			}
			updatedLines.push_back(updatedLine);
		}
		writeFile(updatedLines);
	}
}

void MainWindow::slot_DeleteWord()
{
    //Silinecek edilecek kelime bos ise kullaniciya hata mesaji doner.
    QString deletedWord = ui.leDelete->text();
    if(deletedWord.size() == 0)
    {
        QMessageBox msg;
        msg.setText("This field can not be empty.");
        msg.exec();
    }
    else
    {
         //aranan kelime bulunamadiysa kullanciya hata mesaji doner ve metoddan cikilir.
        if(!doesWordExist(allWords, deletedWord))
        {
            QMessageBox msg;
            msg.setText(deletedWord + " can not find this document.");
            msg.exec();
            return;
        }
        //hata durumlarina dusmeyen durumda asagisi isletilir.
        //replace edilecek kelime tüm kelimelerde aranir ve silinir.
        //tum islemler yapildiktan sonra ayni dosyaya yazdirilir.
        else
        {
            std::vector<QString> updatedLines;
            updatedLines.clear();
            for(size_t index = 0; index < lines.size(); index++)
            {
                QString line = lines[index];
                QString updatedLine;
                auto words = line.split(" ", QString::SkipEmptyParts);
                for (size_t j = 0; j < words.size(); j++)
                {
                    if(words[j] != deletedWord)
                    {
                        updatedLine.append(words[j]);
                        updatedLine.append(" ");
                    }
                    else
                    {
                        continue;
                    }
                }
                 updatedLines.push_back(updatedLine);
            }
            writeFile(updatedLines);

        }
    }

}



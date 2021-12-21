#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QDebug>
#include <QFile>
#include <QFileDialog>
#include <QMessageBox>
#include <QMetaType>
#include <QSet>
#include <QStandardPaths>
#include <QTcpServer>
#include <QTcpSocket>
#include <string>
#include <vector>
using namespace std;

namespace Ui {
class MainWindow;
}

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit MainWindow(QWidget *parent = nullptr);
    ~MainWindow();
    const vector<string> kelimelereAyirici(const string& gelen, const char& ayiriciKarakter);
    void dosya();
    int asama,islem;
    bool var=true;
    string giris;
    struct bilgiler{
        string isim;
        string soyisim;
        string banka;
        string musteriNo;
        double bakiye;
        string kullaniciAdi;
        string parola;
    };
    void yazdir(QString);
    void csvguncelleme(vector <bilgiler>);
    string paraYolla(string);


signals:
    void newMessage(QString);
private slots:
    void newConnection();
    void appendToSocketList(QTcpSocket* socket);

    void readSocket();
    void discardSocket();
    void displayError(QAbstractSocket::SocketError socketError);

    void displayMessage(const QString& str);
    void sendMessage(QTcpSocket* socket);


    void on_pushButton_sendMessage_clicked();


    void refreshComboBox();
private:
    Ui::MainWindow *ui;
    vector <bilgiler> bilgilerVector;
    QTcpServer* m_server;
    QSet<QTcpSocket*> connection_set;

    int aktifKullaniciIndex;
};

#endif // MAINWINDOW_H

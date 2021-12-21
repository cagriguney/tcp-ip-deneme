#include "mainwindow.h"
#include "ui_mainwindow.h"
#include <vector>
#include <sstream>
#include <fstream>
#include <clocale>
#include <istream>
#include <iostream>
#include <string>


using namespace std;

MainWindow::MainWindow(QWidget *parent) : QMainWindow(parent), ui(new Ui::MainWindow)
{

    ui->setupUi(this);
    m_server = new QTcpServer();
    dosya();
    asama=0;
    aktifKullaniciIndex=-1;
    islem=-1;

    if(m_server->listen(QHostAddress::Any, 8080))
    {
       connect(this, &MainWindow::newMessage, this, &MainWindow::displayMessage);
       connect(m_server, &QTcpServer::newConnection, this, &MainWindow::newConnection);
       ui->statusBar->showMessage("Server is listening...");
    }
    else
    {
        QMessageBox::critical(this,"QTCPServer",QString("Unable to start the server: %1.").arg(m_server->errorString()));
        exit(EXIT_FAILURE);
    }

}

MainWindow::~MainWindow()
{
    foreach (QTcpSocket* socket, connection_set)
    {
        socket->close();
        socket->deleteLater();
    }

    m_server->close();
    m_server->deleteLater();

    delete ui;
}

void MainWindow::newConnection()
{
    while (m_server->hasPendingConnections())
        appendToSocketList(m_server->nextPendingConnection());
}

const vector<string> MainWindow::kelimelereAyirici(const string& gelen, const char& ayiriciKarakter) {
    string kelimeOlusturucu{ "" };
    vector<string> sonucVektoru;

    for (auto karakter : gelen) {
        if (karakter != ayiriciKarakter) {
            kelimeOlusturucu += karakter;
        }
        else if (karakter == ayiriciKarakter && kelimeOlusturucu != "") {
            sonucVektoru.push_back(kelimeOlusturucu);
            kelimeOlusturucu = "";
        }
    }

    if (kelimeOlusturucu != "") {
        sonucVektoru.push_back(kelimeOlusturucu);
    }

    return sonucVektoru;
}

void MainWindow::appendToSocketList(QTcpSocket* socket)
{
    connection_set.insert(socket);
    connect(socket, &QTcpSocket::readyRead, this, &MainWindow::readSocket);
    connect(socket, &QTcpSocket::disconnected, this, &MainWindow::discardSocket);
    connect(socket, &QAbstractSocket::errorOccurred, this, &MainWindow::displayError);
    ui->comboBox_receiver->addItem(QString::number(socket->socketDescriptor()));
    displayMessage(QString("INFO :: Client with sockd:%1 has just entered the room").arg(socket->socketDescriptor()));
}

void MainWindow::readSocket()
{
    QTcpSocket* socket = reinterpret_cast<QTcpSocket*>(sender());

    QByteArray buffer;

    QDataStream socketStream(socket);
    socketStream.setVersion(QDataStream::Qt_5_15);

    socketStream.startTransaction();
    socketStream >> buffer;

    if(!socketStream.commitTransaction())
    {
        QString message = QString("%1 :: Waiting for more data to come..").arg(socket->socketDescriptor());
        emit newMessage(message);
        return;
    }

    QString header = buffer.mid(0,128);
    QString fileType = header.split(",")[0].split(":")[1];

    buffer = buffer.mid(128);

    QString message = QString("%1 :: %2").arg("Client").arg(QString::fromStdString(buffer.toStdString()));
    emit newMessage(message);

    if(asama==2){
        if(var){
            islem=stoi(buffer.toStdString());
            cout<<"islem "<<islem<<endl;
        }

        switch(islem){
            case 1:{
            if(var){
                yazdir("Lütfen Cekmek Istediginiz Miktari Giriniz:");
                var=false;
            }
            else{
                    int g=stoi(buffer.toStdString());
                    cout<<g<<"----g is"<<endl;
                    if(g>bilgilerVector[aktifKullaniciIndex].bakiye){
                        yazdir("Bakiye Yetersiz,İslem Gerceklesmedi");
                        asama=0;
                        islem=0;
                    }
                    else{
                        bilgilerVector[aktifKullaniciIndex].bakiye=bilgilerVector[aktifKullaniciIndex].bakiye-g;
                        stringstream dumps;
                        cout<<bilgilerVector[aktifKullaniciIndex].bakiye<<"sondddd"<<endl;
                        csvguncelleme(bilgilerVector);
                        dumps<<"Guncel bakiyeniz: "<<bilgilerVector[aktifKullaniciIndex].bakiye;
                        yazdir(QString::fromStdString(dumps.str()));
                        asama=0;
                        islem=0;
                    }

                }

            break;
            }
            case 2:{
            if(var){
                yazdir("Lutfen yatirmak istediginiz miktari giriniz:");
                var=false;
            }
            else{

                bilgilerVector[aktifKullaniciIndex].bakiye+=stod(buffer.toStdString());
                stringstream dumps2;
                dumps2<<"Guncel bakiyeniz: "<<bilgilerVector[aktifKullaniciIndex].bakiye;
                yazdir(QString::fromStdString(dumps2.str()));
                csvguncelleme(bilgilerVector);
                asama=1;

            }
            break;
            }
            case 3:{
            if(var){
                yazdir("Para yatirmak istediginiz müsterinin isim, soyisim, musteri numarası ve tutarı, aralarına virgul koyarak giriniz");
                var=false;
            }
            else{
                string result=paraYolla(buffer.toStdString());
                yazdir(QString::fromStdString(result));
                asama=1;

            }
            break;
            }
        case 4:{
            asama=0;
            break;
        }
        }

    }

    if(asama==1 && buffer.toStdString()!=""){

        if(bilgilerVector[aktifKullaniciIndex].parola==buffer.toStdString()){
            asama=2;
            stringstream dumpss;
            dumpss<<"\nBakiyeniz: "<<bilgilerVector[aktifKullaniciIndex].bakiye<<"\nSimdi isleminizi seciniz:\n1-Para Cekme\n2-Para Yatirma\n3-Baska Hesaba Para Yatirma\n4-Cikis";
            yazdir(QString::fromStdString(dumpss.str()));
        }
        else{
            yazdir("Sifreniz dogru degil, kontrol edip tekrar giriniz:");
        }
    }

    if(asama==0 && buffer.toStdString()!=""){
        for(int i=0;i<bilgilerVector.size();i++){
            if(bilgilerVector[i].kullaniciAdi==buffer.toStdString()){
                asama=1;
                aktifKullaniciIndex=i;
                yazdir("Sifrenizi Giriniz:");
            }

        }
        if(asama==0) {
            yazdir("Kullanici adi sisteme kayitli değil, lütfen kontrol edip tekrar deneyin");
        }


    buffer.clear();
    }
}



string MainWindow::paraYolla(string total){

    int index=-1;
    vector<string> sonuc=kelimelereAyirici(total,',');

    for(int i=0;i<bilgilerVector.size();i++){
        //cout<<bilgilerVector[i].isim << "---" <<sonuc[0]<<endl;
        if(bilgilerVector[i].isim==sonuc[0]){
            index=i;
        }
    }

    if(index==-1){
        return "Girilen bilgiler hatali/sistemde kayitli degil!";
    }
    if(bilgilerVector[index].bakiye<stoi(sonuc[3])){
        return "İslem gerceklestirilemedi,bakiye yetersiz.";
    }
    if(bilgilerVector[index].soyisim==sonuc[1] && bilgilerVector[index].musteriNo==sonuc[2]){
        int commissionRate=10;
        bilgilerVector[index].bakiye=bilgilerVector[index].bakiye+stoi(sonuc[3]);
        if(bilgilerVector[index].banka==bilgilerVector[aktifKullaniciIndex].banka){

            bilgilerVector[aktifKullaniciIndex].bakiye-=stoi(sonuc[3]);
            csvguncelleme(bilgilerVector);
            return "İslem basarili.";
        }

        else{
            bilgilerVector[aktifKullaniciIndex].bakiye-=(stoi(sonuc[3])+stoi(sonuc[3])*commissionRate/100);
            csvguncelleme(bilgilerVector);
            return "İslem basarili.";
        }
    }
    else
        return "Girilen bilgiler sistemde kayitli degil!";



}

void MainWindow::yazdir(QString a){

    QString receiver = ui->comboBox_receiver->currentText();
    ui->lineEdit_message->setText(a);
    if(receiver=="Broadcast")
    {
        foreach (QTcpSocket* socket,connection_set)
        {
            sendMessage(socket);
        }
    }
    else
    {
        foreach (QTcpSocket* socket,connection_set)
        {
            if(socket->socketDescriptor() == receiver.toLongLong())
            {
                sendMessage(socket);
                break;
            }
        }
    }
    ui->lineEdit_message->clear();

}

void MainWindow::csvguncelleme(vector<bilgiler> bilgivektor){

    ofstream csv;
    csv.open("datalar.csv",ios::trunc); // app yaparsak dosya silinmez altına yazar
    stringstream dump2;
    for(int i=0;i<bilgilerVector.size();i++){

        csv<<bilgilerVector[i].isim<<","<<bilgilerVector[i].soyisim<<","<<bilgilerVector[i].banka<<","
            <<bilgilerVector[i].musteriNo<<","<<bilgilerVector[i].bakiye<<","<<bilgilerVector[i].kullaniciAdi<<","<<bilgilerVector[i].parola<<"\n";

    }

    csv.close();

}

void MainWindow::discardSocket()
{
    QTcpSocket* socket = reinterpret_cast<QTcpSocket*>(sender());
    QSet<QTcpSocket*>::iterator it = connection_set.find(socket);
    if (it != connection_set.end()){
        displayMessage(QString("INFO :: A client has just left the room").arg(socket->socketDescriptor()));
        connection_set.remove(*it);
    }
    refreshComboBox();
    
    socket->deleteLater();
}

void MainWindow::displayError(QAbstractSocket::SocketError socketError)
{
    switch (socketError) {
        case QAbstractSocket::RemoteHostClosedError:
        break;
        case QAbstractSocket::HostNotFoundError:
            QMessageBox::information(this, "QTCPServer", "The host was not found. Please check the host name and port settings.");
        break;
        case QAbstractSocket::ConnectionRefusedError:
            QMessageBox::information(this, "QTCPServer", "The connection was refused by the peer. Make sure QTCPServer is running, and check that the host name and port settings are correct.");
        break;
        default:
            QTcpSocket* socket = qobject_cast<QTcpSocket*>(sender());
            QMessageBox::information(this, "QTCPServer", QString("The following error occurred: %1.").arg(socket->errorString()));
        break;
    }
}

void MainWindow::on_pushButton_sendMessage_clicked()
{
    QString receiver = ui->comboBox_receiver->currentText();

    if(receiver=="Broadcast")
    {
        foreach (QTcpSocket* socket,connection_set)
        {
            sendMessage(socket);
        }
    }
    else
    {
        foreach (QTcpSocket* socket,connection_set)
        {
            if(socket->socketDescriptor() == receiver.toLongLong())
            {
                sendMessage(socket);
                break;
            }
        }
    }
    ui->lineEdit_message->clear();
}


void MainWindow::sendMessage(QTcpSocket* socket)
{
    if(socket)
    {
        if(socket->isOpen())
        {

            QString str = ui->lineEdit_message->text();

            QDataStream socketStream(socket);
            socketStream.setVersion(QDataStream::Qt_5_15);

            QByteArray header;
            header.prepend(QString("fileType:message,fileName:null,fileSize:%1;").arg(str.size()).toUtf8());
            header.resize(128);

            QByteArray byteArray = str.toUtf8();
            byteArray.prepend(header);

            socketStream.setVersion(QDataStream::Qt_5_15);
            socketStream << byteArray;
        }
        else
            QMessageBox::critical(this,"QTCPServer","Socket doesn't seem to be opened");
    }
    else
        QMessageBox::critical(this,"QTCPServer","Not connected");
}

void MainWindow::displayMessage(const QString& str)
{
    ui->textBrowser_receivedMessages->append(str);
}

void MainWindow::refreshComboBox(){
    ui->comboBox_receiver->clear();
    ui->comboBox_receiver->addItem("Broadcast");
    foreach(QTcpSocket* socket, connection_set)
        ui->comboBox_receiver->addItem(QString::number(socket->socketDescriptor()));

}

void MainWindow::dosya(){

    ifstream dosyaniOku;
    dosyaniOku.open("datalar.csv");
    string dosya;
    vector<string>sonucVektoru{};
    string mNo,ad,soyad;

    while (getline(dosyaniOku, dosya)) {

        bilgiler temp;

        vector<string>sonucVektoru{ kelimelereAyirici(dosya,',') };
        temp.isim = sonucVektoru[0];
        temp.soyisim = sonucVektoru[1];
        temp.banka = sonucVektoru[2];
        temp.musteriNo = sonucVektoru[3];
        temp.bakiye = stod(sonucVektoru[4]);
        temp.kullaniciAdi = sonucVektoru[5];
        temp.parola = sonucVektoru[6];
        bilgilerVector.push_back(temp);


    }

    dosyaniOku.close();
}

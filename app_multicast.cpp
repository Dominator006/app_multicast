#include "app_multicast.h"
#include "ui_app_multicast.h"

#include <QtNetwork>
#include <QImage>
#include <QFileDialog>
#include <QBuffer>
#include <QUdpSocket>

App_multicast::App_multicast(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::App_multicast),count_image(0)
{
    ui->setupUi(this);
    ui->pseudo->setText("Anonyme");

  /* création du socket client-server UDP  */
    socket = new QUdpSocket(this);
    socket->setSocketOption(QAbstractSocket::MulticastTtlOption, 1);



    if (!socket->bind(QHostAddress(MULTICAST_ADDRESS), PORT_GROUP , QUdpSocket::ReuseAddressHint|QUdpSocket::ShareAddress))
           exit(EXIT_FAILURE);
    socket->joinMulticastGroup((QHostAddress(MULTICAST_ADDRESS)));

    connect(socket, &QUdpSocket::readyRead, this, &App_multicast::processPendingDatagram);


    //connect(socket,SIGNAL(errorOccurred(QAbstractSocket::SocketError)),this,SLOT(erreurSocket(QAbstractSocket::SocketError)));



}



void App_multicast::boutonEnvoyer_msg_clicked(){
    QByteArray message = ui->message->toPlainText().toUtf8();
    QByteArray datagram = ui->pseudo->text().toUtf8();
    datagram = datagram + " : " + message;
    socket->writeDatagram(datagram, QHostAddress(MULTICAST_ADDRESS), PORT_GROUP);
    ui->message->clear();
}



/* SLOT reception message */
void App_multicast::processPendingDatagram(){

    while (socket->hasPendingDatagrams())
        {
            QByteArray datagram;
            QHostAddress sender;
            quint16 senderPort;
            datagram.resize(socket->pendingDatagramSize());
            socket->readDatagram(datagram.data(), datagram.size(), &sender, &senderPort);
            if (datagram.size() < 2000)
            {
                QStringList msg_split = QString(datagram.data()).split(":");
                            if (msg_split[0] == "Anonyme " and msg_split[1] != "/image#147258!@.")
                            {
                                ui->display_msg->append("Anonyme " + QString::number(senderPort) + " :" + msg_split[1]);
                            } else if (msg_split[0] == "Anonyme" and msg_split[1] == "/image#147258!@.")
                            {
                                ui->display_msg->append("Anonyme " + QString::number(senderPort) + " a envoyé une image.");

                            } else if (msg_split[0] != "Anonyme" and msg_split[1] == "/image#147258!@.")
                            {
                                ui->display_msg->append(msg_split[0] + " a envoyé une image.");

                            } else if (msg_split[0] != "Anonyme" and msg_split[1] == "arejoint#147258!@.")
                            {
                                ui->display_msg->append(msg_split[0] + " a rejoint la discussion !");
                            } else if (msg_split[0] == "Anonyme" and msg_split[1] == "arejoint#147258!@.")
                            {
                                ui->display_msg->append("Anonyme " + QString::number(senderPort) + " a rejoint la discussion !");
                            } else
                            {
                                ui->display_msg->append(datagram.data());
                            }
            } else
            {
                QImage image_temp = get_imagedata_from_byte(datagram.data());
                image_temp.save(QString("./tmp/image%1.png").arg(count_image), "PNG");
                QImage* imgScaled = new QImage;
                *imgScaled = image_temp.scaled(400, 400, Qt::KeepAspectRatio);
                ui->image->setPixmap(QPixmap::fromImage(*imgScaled));
                count_image++;


            }
       }
}




QByteArray  App_multicast::get_imagedata_from_imagefile(const QImage &image)
{
    QByteArray imageData;
    QBuffer buffer(&imageData);
    image.save(&buffer, "JPG");
    imageData = imageData.toBase64();
    return imageData;
}

QImage App_multicast::get_imagedata_from_byte(const QString &data)
{
    QByteArray imageData = QByteArray::fromBase64(data.toLatin1());
    QImage image;
    image.loadFromData(imageData);
    return image;
}



void App_multicast::boutonEnvoyer_img_Clicked()
{
    QString file_name = QFileDialog::getOpenFileName(this, "Image à envoyer", QDir::currentPath(), "Image Files(*.jpg *.png)");
    if (file_name == "")
        return;

    image.load(file_name);
    ui->image->setPixmap(QPixmap::fromImage(image).scaled(ui->image->size()));

    image_file_data = get_imagedata_from_imagefile(image);
    socket->writeDatagram(image_file_data.toLatin1(),image_file_data.size(),QHostAddress(MULTICAST_ADDRESS), PORT_GROUP);

    QByteArray datagram = ui->message->toPlainText().toUtf8();
    datagram += ":/image#147258!@.";
    socket->writeDatagram(datagram, QHostAddress(MULTICAST_ADDRESS), PORT_GROUP);
}





void App_multicast::erreurSocket(QAbstractSocket::SocketError erreur){
    switch(erreur) {

    case QAbstractSocket::HostNotFoundError:

           ui->display_msg->append(tr("<em>ERREUR : le serveur n'a pas pu être trouvé. Vérifiez l'IP et le port.</em>"));

          break;
    case QAbstractSocket::ConnectionRefusedError:
        ui->display_msg->append(tr("<em>ERREUR : le serveur a refusé la connexion. Vérifiez si le programme \"serveur\" a bien été lancé. Vérifiez aussi l'IP et le port.</em>"));
        break;

    case QAbstractSocket::RemoteHostClosedError:
            ui->display_msg->append(tr("<em>ERREUR : le serveur a coupé la connexion.</em>"));
          break;

    default:
       ui->display_msg->append(tr("<em>ERREUR : ") + socket->errorString() + tr("</em>"));

    }
}





//void App_multicast::message_returnPressed()
//{
 //    boutonEnvoyer_msg_clicked();
//}






App_multicast::~App_multicast(){
    delete ui;
}



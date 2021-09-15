using System;
using System.Collections.Generic;
using System.IO.Ports;
using System.Linq;
using System.Text;
using System.Threading.Tasks;
using System.Windows;
using System.Windows.Controls;
using System.Windows.Data;
using System.Windows.Documents;
using System.Windows.Input;
using System.Windows.Media;
using System.Windows.Media.Imaging;
using System.Windows.Navigation;
using System.Windows.Shapes;
using ExtendedSerialPort;
using System.Windows.Threading;


namespace RobotInterface
{
    /// <summary>
    /// Logique d'interaction pour MainWindow.xaml
    /// </summary>

    public partial class MainWindow : Window
    {
        ReliableSerialPort serialPort1;
        DispatcherTimer timerAffichage;

        Robot robot = new Robot();

        public MainWindow()
        {
            InitializeComponent();
            serialPort1 = new ReliableSerialPort("COM11", 115200, Parity.None, 8, StopBits.One);
            serialPort1.DataReceived += SerialPort1_DataReceived;
            serialPort1.Open();

            timerAffichage = new DispatcherTimer();
            timerAffichage.Interval = new TimeSpan(0, 0, 0, 0, 100);
            timerAffichage.Tick += TimerAffichage_Tick;
            timerAffichage.Start();

            robot.receivedText = "";
        }

        public void SerialPort1_DataReceived(object sender, DataReceivedArgs e)
        {
            int i;
            robot.receivedText += Encoding.UTF8.GetString(e.Data, 0, e.Data.Length);
            for (i = 0; i < e.Data.Length; i++)
            {
                DecodeMessage(e.Data[i]);
                //robot.byteListReceived.Enqueue(e.Data[i]);
            }
        }

        public void TimerAffichage_Tick(object sender, EventArgs e)
        {
            //if (robot.receivedText != "")
            //{
            //    textBoxReception.Text = textBoxReception.Text + robot.receivedText;
            //    robot.receivedText = "";
            //}
            while (robot.byteListReceived.Count != 0)
            {
                byte byteReceived = robot.byteListReceived.Dequeue();
                //string test;
                //test = "0X" + byteReceived.ToString("X2") + " "; // () = décimal / ("Xn") = héxadécimal avec minimum n caractères après le "Ox"
                //textBoxReception.Text += test;
                DecodeMessage(byteReceived);
            }
        }

        void SendMessage()
        {
            serialPort1.WriteLine(textBoxEmission.Text);
            textBoxEmission.Text = "";
        }

        public void buttonEnvoyer_Click(object sender, RoutedEventArgs e)
        {
            /*if (buttonEnvoyer.Background == Brushes.RoyalBlue)
                buttonEnvoyer.Background = Brushes.Beige;
            else
                buttonEnvoyer.Background = Brushes.RoyalBlue;*/

            SendMessage();
            textBoxReception.Text = textBoxReception.Text + "\n";
        }

        public void textBoxEmission_KeyUp(object sender, KeyEventArgs e)
        {
            if (e.Key == Key.Enter)
            {
                SendMessage();
            }
        }

        public void buttonEffacer_Click(object sender, RoutedEventArgs e)
        {
            textBoxReception.Text = "";
        }

        public void buttonTest_Click(object sender, RoutedEventArgs e)
        {
            //int i;
            //byte[] byteList = new byte[20];
            //for (i = 0; i < 20; i++)
            //{
            //    byteList[i] = (byte)(2 * i);
            //}
            //serialPort1.Write(byteList, 0, byteList.Length);

            //byte[] chaine = Encoding.ASCII.GetBytes("Bonjour");
            //UartEncodeAndSendMessage(0x0080, chaine.Length, chaine);

            byte[] chaine = new byte[2];
            chaine[0] = Convert.ToByte(2);
            chaine[1] = Convert.ToByte(1);
            UartEncodeAndSendMessage(0x0020, chaine.Length, chaine);

            //byte[] chaine = new byte[3];
            //chaine[0] = Convert.ToByte(40);
            //chaine[1] = Convert.ToByte(50);
            //chaine[2] = Convert.ToByte(65);
            //UartEncodeAndSendMessage(0x0030, chaine.Length, chaine);

            //byte[] chaine = new byte[2];
            //chaine[0] = Convert.ToByte(70);
            //chaine[1] = Convert.ToByte(15);
            //UartEncodeAndSendMessage(0x0040, chaine.Length, chaine);
        }

        byte CalculateChecksum(int msgFunction, int msgPayloadLength, byte[] msgPayload)
        {
            byte checksum = 0xFE;
            checksum ^= (byte)(msgFunction >> 8);
            checksum ^= (byte)(msgFunction >> 0);
            checksum ^= (byte)(msgPayloadLength >> 8);
            checksum ^= (byte)(msgPayloadLength >> 0);
            for (int i = 0; i < msgPayloadLength; i++)
            {
                checksum ^= msgPayload[i];
            }
            return checksum;
        }

        void UartEncodeAndSendMessage(int msgFunction, int msgPayloadLength, byte[] msgPayload)
        {
            byte checksum = CalculateChecksum(msgFunction, msgPayloadLength, msgPayload);
            byte[] msg = new byte[6 + msgPayloadLength];
            int pos = 0;
            msg[pos++] = 0xFE;
            msg[pos++] = (byte)(msgFunction >> 8);
            msg[pos++] = (byte)(msgFunction >> 0);
            msg[pos++] = (byte)(msgPayloadLength >> 8);
            msg[pos++] = (byte)(msgPayloadLength >> 0);
            for (int i = 0; i < msgPayloadLength; i++)
            {
                msg[pos++] = msgPayload[i];
            }
            msg[pos++] = checksum;
            serialPort1.Write(msg, 0, 6 + msgPayloadLength);
        }

        public enum StateReception
        {
            Waiting,
            FunctionMSB,
            FunctionLSB,
            PayloadLengthMSB,
            PayloadLengthLSB,
            Payload,
            CheckSum
        }

        StateReception rcvState = StateReception.Waiting;
        int msgDecodedFunction = 0;
        int msgDecodedPayloadLength = 0;
        byte[] msgDecodedPayload;
        int msgDecodedPayloadIndex = 0;

        private void DecodeMessage(byte c)
        {
            switch (rcvState)
            {
                case StateReception.Waiting:
                    if (c == 0xFE)
                    {
                        rcvState = StateReception.FunctionMSB;
                        msgDecodedFunction = 0;

                    }
                    break;

                case StateReception.FunctionMSB:
                    msgDecodedFunction = c << 8;
                    rcvState = StateReception.FunctionLSB;
                    break;

                case StateReception.FunctionLSB:
                    msgDecodedFunction += c << 0;
                    rcvState = StateReception.PayloadLengthMSB;
                    break;

                case StateReception.PayloadLengthMSB:
                    msgDecodedPayloadLength = c << 8;
                    rcvState = StateReception.PayloadLengthLSB;
                    break;

                case StateReception.PayloadLengthLSB:
                    msgDecodedPayloadLength += c << 0;
                    if (msgDecodedPayloadLength == 0)
                    {
                        rcvState = StateReception.Waiting;
                    }
                    else
                    {
                        rcvState = StateReception.Payload;
                        msgDecodedPayload = new byte[msgDecodedPayloadLength];
                        msgDecodedPayloadIndex = 0;
                    }
                    break;

                case StateReception.Payload:
                    msgDecodedPayload[msgDecodedPayloadIndex++] = c;
                    if (msgDecodedPayloadIndex >= msgDecodedPayloadLength)
                    {
                        rcvState = StateReception.CheckSum;
                    }
                    break;

                case StateReception.CheckSum:
                    byte calculatedChecksum = CalculateChecksum(msgDecodedFunction, msgDecodedPayloadLength, msgDecodedPayload);
                    byte receivedChecksum = c;
                    if (calculatedChecksum == receivedChecksum)
                    {
                        //textBoxReception.Text += "Message valide" + "\n";
                        Dispatcher.Invoke(delegate { ProcessDecodedMessage(msgDecodedFunction, msgDecodedPayloadLength, msgDecodedPayload); });
                    }
                    else
                    {
                        Dispatcher.Invoke(delegate { textBoxReception.Text += "Message invalide + \n"; });
                    }
                    rcvState = StateReception.Waiting;
                    break;

                default:
                    rcvState = StateReception.Waiting;
                    break;
            }
        }

        public enum Function
        {
            Texte = 0x0080,
            LED = 0x0020,
            IR = 0x0030,
            Moteurs = 0x0040,
            StateRobot = 0x0050
            
        }

        public enum StateRobot
        {
            STATE_ATTENTE = 0,
            STATE_ATTENTE_EN_COURS = 1,
            STATE_AVANCE = 2,
            STATE_AVANCE_EN_COURS = 3,
            STATE_TOURNE_PEU_GAUCHE = 4,
            STATE_TOURNE_PEU_GAUCHE_EN_COURS = 5,
            STATE_TOURNE_PEU_DROITE = 6,
            STATE_TOURNE_PEU_DROITE_EN_COURS = 7,
            STATE_TOURNE_GAUCHE = 8,
            STATE_TOURNE_GAUCHE_EN_COURS = 9,
            STATE_TOURNE_DROITE = 10,
            STATE_TOURNE_DROITE_EN_COURS = 11,
            STATE_TOURNE_BEAUCOUP_GAUCHE = 12,
            STATE_TOURNE_BEAUCOUP_GAUCHE_EN_COURS = 13,
            STATE_TOURNE_BEAUCOUP_DROITE = 14,
            STATE_TOURNE_BEAUCOUP_DROITE_EN_COURS = 15,
            STATE_TOURNE_SUR_PLACE_GAUCHE = 16,
            STATE_TOURNE_SUR_PLACE_GAUCHE_EN_COURS = 17,
            STATE_TOURNE_SUR_PLACE_DROITE = 18,
            STATE_TOURNE_SUR_PLACE_DROITE_EN_COURS = 19,
            STATE_ARRET = 20,
            STATE_ARRET_EN_COURS = 21,
            STATE_RECULE = 22,
            STATE_RECULE_EN_COURS = 23,
            STATE_AVANCE_LENT = 24,
            STATE_AVANCE_LENT_EN_COURS = 25,
            STATE_AVANCE_RAPIDE = 26,
            STATE_AVANCE_RAPIDE_EN_COURS = 27,
            STATE_TURBO = 28,
            STATE_TURBO_EN_COURS = 29
        }

        void ProcessDecodedMessage(int msgFunction, int msgPayloadLength, byte[] msgPayload)
        {
            switch (msgFunction)
            {
                case (int)Function.Texte:
                    textBoxReception.Text += Encoding.UTF8.GetString(msgPayload) + '\n';
                    break;

                case (int)Function.LED:
                    byte LED = msgPayload[0];
                    byte etatLED = msgPayload[1];
                    switch (LED)
                    {
                        case 1:
                            if (etatLED == 1)
                            {
                                checkBoxLEDOrange.IsChecked = Convert.ToBoolean(1);
                            }
                            else
                            {
                                checkBoxLEDOrange.IsChecked = Convert.ToBoolean(0);
                            }
                            break;

                        case 2:
                            if (etatLED == 1)
                            {
                                checkBoxLEDBleue.IsChecked = Convert.ToBoolean(1);
                            }
                            else
                            {
                                checkBoxLEDBleue.IsChecked = Convert.ToBoolean(0);
                            }
                            break;

                        case 3:
                            if (etatLED == 1)
                            {
                                checkBoxLEDBlanche.IsChecked = Convert.ToBoolean(1);
                            }
                            else
                            {
                                checkBoxLEDBlanche.IsChecked = Convert.ToBoolean(0);
                            }
                            break;
                    }
                    break;

                case (int)Function.IR:
                    textBoxIRG.Text = msgPayload[0] + " cm";
                    textBoxIRC.Text = msgPayload[1] + " cm";
                    textBoxIRD.Text = msgPayload[2] + " cm";
                    break;

                case (int)Function.Moteurs:
                    textBoxVG.Text = msgPayload[0] + " %";
                    textBoxVD.Text = msgPayload[1] + " %";
                    break;

                case (int)Function.StateRobot:
                    int instant = (((int)msgPayload[1]) << 24) + (((int)msgPayload[2]) << 16) + (((int)msgPayload[3]) << 8) + ((int)msgPayload[4]);
                    textBoxEtape.Text = ((StateRobot)(msgPayload[0])).ToString() + "\n" + instant.ToString() + " ms";
                    break;
            }
        }
    }
}

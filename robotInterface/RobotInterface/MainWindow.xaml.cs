using ExtendedSerialPort;
using MouseKeyboardActivityMonitor;
using MouseKeyboardActivityMonitor.WinApi;
using System;
using System.IO.Ports;
using System.Text;
using System.Windows;
using System.Windows.Forms;
using System.Windows.Input;
using System.Windows.Threading;
using Utilities;

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

        private readonly KeyboardHookListener m_KeyboardHookManager;

        public MainWindow()
        {
            InitializeComponent();
            serialPort1 = new ReliableSerialPort("COM6", 115200, Parity.None, 8, StopBits.One);
            serialPort1.DataReceived += SerialPort1_DataReceived;
            serialPort1.Open();

            timerAffichage = new DispatcherTimer();
            timerAffichage.Interval = new TimeSpan(0, 0, 0, 0, 100);
            timerAffichage.Tick += TimerAffichage_Tick;
            timerAffichage.Start();

            robot.receivedText = "";

            m_KeyboardHookManager = new KeyboardHookListener(new GlobalHooker());
            m_KeyboardHookManager.Enabled = true;
            m_KeyboardHookManager.KeyDown += HookManager_KeyDown;
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
            string message = textBoxEmission.Text.TrimEnd('\n');
            serialPort1.WriteLine(message);
            byte[] chaine = Encoding.UTF8.GetBytes(message);
            UartEncodeAndSendMessage(0x0080, chaine.Length, chaine);
            textBoxEmission.Text = "";
        }

        public void buttonEnvoyer_Click(object sender, RoutedEventArgs e)
        {
            /*if (buttonEnvoyer.Background == Brushes.RoyalBlue)
                buttonEnvoyer.Background = Brushes.Beige;
            else
                buttonEnvoyer.Background = Brushes.RoyalBlue;*/
            //textBoxReception.Text = textBoxReception.Text + "\n";
            SendMessage();
        }

        public void textBoxEmission_KeyUp(object sender, System.Windows.Input.KeyEventArgs e)
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

            //byte[] chaine = new byte[2];
            //chaine[0] = Convert.ToByte(2);
            //chaine[1] = Convert.ToByte(1);
            //UartEncodeAndSendMessage(0x0020, chaine.Length, chaine);

            //byte[] chaine = new byte[3];
            //chaine[0] = Convert.ToByte(40);
            //chaine[1] = Convert.ToByte(50);
            //chaine[2] = Convert.ToByte(65);
            //UartEncodeAndSendMessage(0x0030, chaine.Length, chaine);

            //byte[] chaine = new byte[2];
            //chaine[0] = Convert.ToByte(70);
            //chaine[1] = Convert.ToByte(15);
            //UartEncodeAndSendMessage(0x0040, chaine.Length, chaine);

            EnvoiAsservissement(sender, e);
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
                        Dispatcher.Invoke(delegate { textBoxReception.Text += "Message invalide \n"; });
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
            StateRobot = 0x0050,
            Set_Robot_State = 0x0051,
            Set_Robot_Manual_Control = 0x0052,
            Position_Data = 0x0061,
            Asservissement = 0x0070
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

                case (int)Function.Set_Robot_State:
                    break;

                case (int)Function.Position_Data:
                    //int offset = 0;
                    //instant = (((int)msgPayload[1]) << 24) + (((int)msgPayload[2]) << 16) + (((int)msgPayload[3]) << 8) + ((int)msgPayload[4]);

                    byte[] tab = msgPayload.GetRange(4, 4);
                    robot.positionXOdo = tab.GetFloat();

                    tab = msgPayload.GetRange(8, 4);
                    robot.positionYOdo = tab.GetFloat();

                    tab = msgPayload.GetRange(12, 4);
                    robot.positionAngleRadOdo = tab.GetFloat();

                    tab = msgPayload.GetRange(16, 4);
                    robot.positionVitesseLinOdo = tab.GetFloat();

                    tab = msgPayload.GetRange(20, 4);
                    robot.positionVitesseAngOdo = tab.GetFloat();

                    textBoxPosX.Text = robot.positionXOdo.ToString();
                    textBoxPosY.Text = robot.positionYOdo.ToString();
                    textBoxAngle.Text = robot.positionAngleRadOdo.ToString();
                    textBoxVitesseLin.Text = robot.positionVitesseLinOdo.ToString();
                    textBoxVitesseAng.Text = robot.positionVitesseAngOdo.ToString();

                    asservSpeedDisplay.UpdatePolarOdometrySpeed(robot.positionVitesseLinOdo, robot.positionVitesseAngOdo);
                    break;

                case (int)Function.Asservissement:

                    int nb_octet = 0;

                    double consigneX;
                    double consigneTheta;
                    double valueX;
                    double valueTheta;
                    double errorX;
                    double errorTheta;
                    //double commandX;
                    //double commandTheta;

                    double corrPX;
                    double corrPTheta;
                    double corrIX;
                    double corrITheta;
                    double corrDX;
                    double corrDTheta;

                    double KpX;
                    double KpTheta;
                    double KiX;
                    double KiTheta;
                    double KdX;
                    double KdTheta;

                    double corrLimitPX;
                    double corrLimitPTheta;
                    double corrLimitIX;
                    double corrLimitITheta;
                    double corrLimitDX;
                    double corrLimitDTheta;

                    consigneX = BitConverter.ToSingle(msgPayload, nb_octet);
                    nb_octet += 4;
                    consigneTheta = BitConverter.ToSingle(msgPayload, nb_octet);
                    nb_octet += +4;
                    valueX = BitConverter.ToSingle(msgPayload, nb_octet);
                    nb_octet += 4;
                    valueTheta = BitConverter.ToSingle(msgPayload, nb_octet);
                    nb_octet += 4;
                    errorX = BitConverter.ToSingle(msgPayload, nb_octet);
                    nb_octet += 4;
                    errorTheta = BitConverter.ToSingle(msgPayload, nb_octet);
                    //tabl = msgPayload.GetRange(nb_octet, 4);
                    //nb_octet += 4;
                    //commandX = tabl.GetFloat();
                    //tabl = msgPayload.GetRange(nb_octet, 4);
                    //nb_octet += 4;
                    //commandTheta = tabl.GetFloat();

                    //------------------- corrPX, corrPTheta, corrIX, corrITheta, corrDX, corrDTheta
                    nb_octet += 4;
                    corrPX = BitConverter.ToSingle(msgPayload, nb_octet);
                    nb_octet += 4;
                    corrPTheta = BitConverter.ToSingle(msgPayload, nb_octet);
                    nb_octet += 4;
                    corrIX = BitConverter.ToSingle(msgPayload, nb_octet);
                    nb_octet += 4;
                    corrITheta = BitConverter.ToSingle(msgPayload, nb_octet);
                    nb_octet += 4;
                    corrDX = BitConverter.ToSingle(msgPayload, nb_octet);
                    nb_octet += 4;
                    corrDTheta = BitConverter.ToSingle(msgPayload, nb_octet);

                    //------------------- KpX, KpTheta, KiX, KiTheta, KdX, KdTheta
                    nb_octet += 4;
                    KpX = BitConverter.ToSingle(msgPayload, nb_octet);
                    nb_octet += 4;
                    KpTheta = BitConverter.ToSingle(msgPayload, nb_octet);
                    nb_octet += 4;
                    KiX = BitConverter.ToSingle(msgPayload, nb_octet);
                    nb_octet += 4;
                    KiTheta = BitConverter.ToSingle(msgPayload, nb_octet);
                    nb_octet += 4;
                    KdX = BitConverter.ToSingle(msgPayload, nb_octet);
                    nb_octet += 4;
                    KdTheta = BitConverter.ToSingle(msgPayload, nb_octet);

                    //------------------- corrLimitPX, corrLimitPTheta, corrLimitIX, corrLimitITheta, corrLimitDX, corrLimitDTheta
                    nb_octet += 4;
                    corrLimitPX = BitConverter.ToSingle(msgPayload, nb_octet);
                    nb_octet += 4;
                    corrLimitPTheta = BitConverter.ToSingle(msgPayload, nb_octet);
                    nb_octet += 4;
                    corrLimitIX = BitConverter.ToSingle(msgPayload, nb_octet);
                    nb_octet += 4;
                    corrLimitITheta = BitConverter.ToSingle(msgPayload, nb_octet);
                    nb_octet += 4;
                    corrLimitDX = BitConverter.ToSingle(msgPayload, nb_octet);
                    nb_octet += 4;
                    corrLimitDTheta = BitConverter.ToSingle(msgPayload, nb_octet);

                    asservSpeedDisplay.UpdatePolarSpeedConsigneValues(consigneX, consigneTheta);
                    //asservSpeedDisplay.UpdatePolarSpeedCommandValues(commandX, commandTheta);
                    //asservSpeedDisplay.UpdatePolarOdometrySpeed(valueX, valueTheta);
                    asservSpeedDisplay.UpdatePolarSpeedErrorValues(errorX, errorTheta);
                    asservSpeedDisplay.UpdatePolarSpeedCorrectionValues(corrPX, corrPTheta, corrIX, corrITheta, corrDX, corrDTheta);
                    asservSpeedDisplay.UpdatePolarSpeedCorrectionGains(KpX, KpTheta, KiX, KiTheta, KdX, KdTheta);
                    asservSpeedDisplay.UpdatePolarSpeedCorrectionLimits(corrLimitPX, corrLimitPTheta, corrLimitIX, corrLimitITheta, corrLimitDX, corrLimitDTheta);
                    //asservSpeedDisplay.UpdatePolarSpeedConsigneValues(consigneX, consigneTheta);
                    break;
            }
        }

        private void EnvoiAsservissement(object sender, RoutedEventArgs e)
        {
            int pos = 0;
            double LinAng = 1;
            double Kp = 1;
            double Ki = 2;
            double Kd = 3;
            double KpMax = 4;
            double KiMax = 5;
            double KdMax = 6;

            int msgFunction = (int)Function.Asservissement;
            byte[] msgPayload = new byte[28];
            var vLinAng = BitConverter.GetBytes((float)LinAng);
            vLinAng.CopyTo(msgPayload, pos);
            var vKp = BitConverter.GetBytes((float)Kp);
            vKp.CopyTo(msgPayload, pos += 4);
            var vKi = BitConverter.GetBytes((float)Ki);
            vKi.CopyTo(msgPayload, pos += 4);
            var vKd = BitConverter.GetBytes((float)Kd);
            vKd.CopyTo(msgPayload, pos += 4);
            var vKpMax = BitConverter.GetBytes((float)KpMax);
            vKpMax.CopyTo(msgPayload, pos += 4);
            var vKiMax = BitConverter.GetBytes((float)KiMax);
            vKiMax.CopyTo(msgPayload, pos += 4);
            var vKdMax = BitConverter.GetBytes((float)KdMax);
            vKdMax.CopyTo(msgPayload, pos += 4);
            int msgPayloadLength = msgPayload.Length;
            UartEncodeAndSendMessage(msgFunction, msgPayloadLength, msgPayload);
        }


        bool autoControlActivated = true;

        public void buttonAutoManuel_Click(object sender, RoutedEventArgs e)
        {
            int Set_Robot_Manual_Control = 0x0052;
            byte[] autoControl = new byte[1];

            autoControlActivated = !autoControlActivated;
            if (autoControlActivated == true)
            {
                buttonAutoManuel.Content = "Automatique";
                autoControl[0] = 1;
                UartEncodeAndSendMessage(Set_Robot_Manual_Control, 1, new byte[] { 1 });
            }
            else
            {
                buttonAutoManuel.Content = "Manuel";
                autoControl[0] = 0;
                UartEncodeAndSendMessage(Set_Robot_Manual_Control, 1, new byte[] { 0 });
            }
        }

        private void HookManager_KeyDown(object sender, System.Windows.Forms.KeyEventArgs e)
        {
            if (autoControlActivated == false)
            {
                switch (e.KeyCode)
                {
                    case Keys.Left:
                        UartEncodeAndSendMessage(0x0051, 1, new byte[] { (byte)StateRobot.STATE_TOURNE_SUR_PLACE_GAUCHE });
                        break;
                    case Keys.Right:
                        UartEncodeAndSendMessage(0x0051, 1, new byte[] { (byte)StateRobot.STATE_TOURNE_SUR_PLACE_DROITE });
                        break;
                    case Keys.Up:
                        UartEncodeAndSendMessage(0x0051, 1, new byte[] { (byte)StateRobot.STATE_AVANCE });
                        break;
                    case Keys.Down:
                        UartEncodeAndSendMessage(0x0051, 1, new byte[] { (byte)StateRobot.STATE_ARRET });
                        break;
                    case Keys.PageDown:
                        UartEncodeAndSendMessage(0x0051, 1, new byte[] { (byte)StateRobot.STATE_RECULE });
                        break;
                }
            }
        }

    }
}

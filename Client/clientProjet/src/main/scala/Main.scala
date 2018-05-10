import java.net._
import java.io._
import scala.io._
import java.awt.Color
import swing._
import scala.swing.event.Key
import scala.swing.event.KeyPressed
import scala.swing.event.ButtonClicked
import scala.swing.BorderPanel.Position._

class UI(out: PrintStream, pseudo: String) extends MainFrame
{
    title = "Connecté en tant que : "+pseudo

    var bestScore = 0
    var actualScore = 0
    var lastScore = 0

    val bestScoreLabel = new Label {font = new Font("Ariel", java.awt.Font.ITALIC, 30);text = "Meilleur Score : "+bestScore}
    val actualScoreLabel = new Label {font = new Font("Ariel", java.awt.Font.ITALIC, 30);text = "Score : "+actualScore}
    val lastScoreLabel = new Label {font = new Font("Ariel", java.awt.Font.ITALIC, 30);text = "Dernier Score : "+lastScore}

    var adja = List[Button]()
    var grille = List[Button]()

    var dejaUtiliser = List[AbstractButton]()
    val timer = new Label {font = new Font("Ariel", java.awt.Font.ITALIC, 50)}
    val chatInput = new TextField()

    var trajectoire = ""

    val motProposer = new Button
    {
        background = Color.white
        font = new Font("Ariel", java.awt.Font.ITALIC, 50)
        border = Swing.EmptyBorder(0,0,0,0)
    }

    val resetButton = new Button("Reset")
    {
        background = Color.white
        font = new Font("Ariel", java.awt.Font.ITALIC, 50)
        border = Swing.EmptyBorder(0,0,0,0)
    }

    val chatZone = new TextArea
    {
        font = new Font("Ariel", java.awt.Font.ITALIC, 20)
        rows = 10
        lineWrap = true
        wordWrap = true
        editable = false
    }

    val scrollableChatZone = new ScrollPane(chatZone){border = Swing.EmptyBorder(0,0,0,0)}

    for(i <- 0 to 15)
    {
        val tmp = new Button
        {
            font = new Font("Ariel", java.awt.Font.ITALIC, 50)
            background = Color.white
            border = Swing.EmptyBorder(0,0,0,0)
        }

        listenTo(tmp)

        grille = tmp::grille
    }

    var grilleGridPanel = new GridPanel(4,4)
    {
        for(i <- 0 to 15)
        {
            contents += grille.apply(i)
        }

        background = Color.white
    }

    val scoreGridPanel = new GridPanel(1,3)
    {
        contents += lastScoreLabel
        contents += actualScoreLabel
        contents += bestScoreLabel

        background = Color.white
    }

    val leftGridPanel = new GridPanel(3,1)
    {
        contents += motProposer
        contents += resetButton
        contents += timer

        background = Color.white
    }

    val mainGridPanel = new GridPanel(1,3)
    {
        contents += leftGridPanel
        contents += grilleGridPanel
        contents += scrollableChatZone

        background = Color.white
    }

    contents = new BorderPanel
    {
        layout(scoreGridPanel) = North
        layout(mainGridPanel) = Center
        layout(chatInput) = South
    }

    listenTo(chatInput.keys)
    listenTo(motProposer)
    listenTo(resetButton)

    reactions +=
    {
        case KeyPressed(_, Key.Enter, _, _) =>
        {
            out.println("ENVOI/"+chatInput.text+'/')
            chatInput.text = ""
        }
        case ButtonClicked(b) =>
        {
            if(grille.contains(b))
            {
                b.enabled = false
                motProposer.text += b.text
                dejaUtiliser = b::dejaUtiliser
                trajectoire += emplacement(b)
                adja = adjacent(b)
                for(g <- grille)
                {
                    if(adja.contains(g) && !dejaUtiliser.contains(g)) g.enabled = true
                    else g.enabled = false
                }
            }
            else if(b == motProposer)
            {
                dejaUtiliser = Nil
                out.println("TROUVE/"+b.text+'/'+trajectoire+"/\0")
                trajectoire = ""
                b.text = ""
                b.repaint
                for(g <- grille)
                {
                    g.enabled = true
                }
            }
            else if(b == resetButton)
            {
                dejaUtiliser = Nil
                trajectoire = ""
                motProposer.text = ""
                motProposer.repaint
                for(g <- grille)
                {
                    g.enabled = true
                }
            }
        }
    }

    size = new Dimension(1000, 600)

    def updateGrille(nouvelleGrille: String): Unit =
    {
        for(i <- 0 to 15)
        {
            grille.apply(i).text = nouvelleGrille.apply(i)+""
            grille.apply(i).repaint
        }

        lastScore = actualScore;lastScoreLabel.text = "Dernier Score : "+lastScore;lastScoreLabel.repaint
        actualScore = 0;actualScoreLabel.text = "Score : "+actualScore;actualScoreLabel.repaint
    }

    def updateTimer(nouveauTimer: String): Unit =
    {
        if(nouveauTimer.last == '/') timer.text = nouveauTimer.init
        else timer.text = nouveauTimer
        timer.repaint
    }

    def updateScore(nouveauScore: String): Unit =
    {
        var tmp = 0

        if(nouveauScore.head != '/')
        {
            tmp = nouveauScore.init.tail.tail.toInt
        }
        else
        {
            tmp = nouveauScore.init.tail.toInt
        }

        if(tmp > bestScore) bestScore = tmp;bestScoreLabel.text = "Meilleur Score : "+bestScore;bestScoreLabel.repaint
        actualScore += tmp;actualScoreLabel.text = "Score : "+actualScore;actualScoreLabel.repaint
    }

    def nouveauMessage(msg: String): Unit =
    {
        var tmp = false
        var message = ""
        var pseudo = ""

        for(c <- msg)
        {
            if(c == '/') tmp = true

            if(tmp) pseudo = pseudo:+c
            else message = message:+c
        }

        pseudo = pseudo.tail

        chatZone.append(pseudo+" : "+message+'\n')
    }

    def adjacent(b: AbstractButton): List[Button] =
    {
        grille.indexOf(b) match
        {
            case 0 => grille.apply(1)::grille.apply(4)::grille.apply(5)::Nil
            case 1 => grille.apply(0)::grille.apply(2)::grille.apply(4)::grille.apply(5)::grille.apply(6)::Nil
            case 2 => grille.apply(1)::grille.apply(3)::grille.apply(5)::grille.apply(6)::grille.apply(7)::Nil
            case 3 => grille.apply(2)::grille.apply(6)::grille.apply(7)::Nil
            case 4 => grille.apply(0)::grille.apply(1)::grille.apply(5)::grille.apply(8)::grille.apply(9)::Nil
            case 7 => grille.apply(2)::grille.apply(3)::grille.apply(6)::grille.apply(10)::grille.apply(11)::Nil
            case 8 => grille.apply(4)::grille.apply(5)::grille.apply(9)::grille.apply(12)::grille.apply(13)::Nil
            case 11 => grille.apply(6)::grille.apply(7)::grille.apply(10)::grille.apply(14)::grille.apply(15)::Nil
            case 12 => grille.apply(8)::grille.apply(9)::grille.apply(13)::Nil
            case 13 => grille.apply(8)::grille.apply(9)::grille.apply(10)::grille.apply(12)::grille.apply(14)::Nil
            case 14 => grille.apply(9)::grille.apply(10)::grille.apply(11)::grille.apply(13)::grille.apply(15)::Nil
            case 15 => grille.apply(10)::grille.apply(11)::grille.apply(14)::Nil
            case i =>
            grille.apply(i-5)::grille.apply(i-4)::grille.apply(i-3)::grille.apply(i-1)::grille.apply(i+1)::grille.apply(i+3)::grille.apply(i+4)::grille.apply(i+5)::Nil
        }
    }

    def emplacement(b: AbstractButton): String =
    {
        grille.indexOf(b) match
        {
            case 0 => "A1"
            case 1 => "A2"
            case 2 => "A3"
            case 3 => "A4"
            case 4 => "B1"
            case 5 => "B2"
            case 6 => "B3"
            case 7 => "B4"
            case 8 => "C1"
            case 9 => "C2"
            case 10 => "C3"
            case 11 => "C4"
            case 12 => "D1"
            case 13 => "D2"
            case 14 => "D3"
            case 15 => "D4"
        }
    }
}

object MainApp
{
    def main(args: Array[String]): Unit =
    {
        print("Veuillez entrer votre nom d'utilisateur : ")
        var pseudo = scala.io.StdIn.readLine()

        print("Veuillez entrer le nom/adresse du serveur : ")
        val serveur = scala.io.StdIn.readLine()

        print("Veuillez entrer le port du serveur : ")
        val port = scala.io.StdIn.readInt()

        val socket = new Socket(InetAddress.getByName(serveur), port)
        var in = new BufferedSource(socket.getInputStream).getLines
        val out = new PrintStream(socket.getOutputStream)

        out.println("CONNEXION/"+pseudo+"/")

        var tmp = in.next

        while(tmp == "CONNEXION/BADPSEUDO/")
        {
            print("Le pseudo que vous avez entrer est deja pris, veuillez en choisir un autre : ")
            pseudo = scala.io.StdIn.readLine()
            out.println(pseudo)
            tmp = in.next
        }

        val ui = new UI(out, pseudo)
        ui.visible = true
        ui.background = Color.white
        ui.resizable = false

        while(true)
        {
            println(tmp)
            if(tmp.take(12) == "TOUR/tirage/") ui.updateGrille(tmp.slice(12,tmp.size - 1))
            else if(tmp.take(13) == "TOUR/newTime/")ui.updateTimer(tmp.slice(13,tmp.size - 1))
            else if(tmp.take(8) == "MVALIDE/")ui.updateScore(tmp.takeRight(4))
            else if(tmp.take(10) == "RECEPTION/")ui.nouveauMessage(tmp.drop(10))
            tmp = in.next
        }
    }
}

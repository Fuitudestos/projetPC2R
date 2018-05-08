import java.net._
import java.io._
import scala.io._
import java.awt.Color
import swing._
import scala.swing.event.Key
import scala.swing.event.KeyPressed
import scala.swing.BorderPanel.Position._

class UI(val out: PrintStream) extends MainFrame
{
    var grille = List[Button]()
    val timer = new Label {font = new Font("Ariel", java.awt.Font.ITALIC, 50)}
    val motProposer = new Label {font = new Font("Ariel", java.awt.Font.ITALIC, 50)}
    val chatInput = new TextField()
    val resetButton = new Button("Reset")

    resetButton.background = Color.white
    resetButton.font = new Font("Ariel", java.awt.Font.ITALIC, 50)
    resetButton.border = Swing.EmptyBorder(0,0,0,0)

    val chatZone = new TextArea
    {
        rows = 10
        editable = false
    }

    for(i <- 0 to 15)
    {
        grille = new Button
        {
            font = new Font("Ariel", java.awt.Font.ITALIC, 50)
            background = Color.white
            border = Swing.EmptyBorder(0,0,0,0)
        }::grille
    }

    var grilleGridPanel = new GridPanel(4,4)
    {
        for(i <- 0 to 15)
        {
            contents += grille.apply(i)
        }

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
        contents += chatZone

        background = Color.white
    }

    contents = new BorderPanel
    {
        layout(mainGridPanel) = Center
        layout(chatInput) = South
    }

    listenTo(chatInput.keys)

    reactions +=
    {
        case KeyPressed(_, Key.Enter, _, _) => out.println(chatInput.text)
    }

    size = new Dimension(1000, 600)

    def updateGrille(nouvelleGrille: String): Unit =
    {
        for(i <- 0 to 15)
        {
            grille.apply(i).text = nouvelleGrille.apply(i)+""
            grille.apply(i).repaint
        }
    }

    def updateTimer(nouveauTimer: String): Unit =
    {
        timer.text = nouveauTimer
        timer.repaint
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

        val ui = new UI(out)
        ui.visible = true
        ui.title = "Connecté en tant que : "+pseudo
        ui.background = Color.white
        ui.resizable = false

        while(true)
        {
            println(tmp)
            if(tmp.take(12) == "TOUR/tirage/") ui.updateGrille(tmp.slice(12,tmp.size - 1))
            else if(tmp.take(13) == "TOUR/newTime/")ui.updateTimer(tmp.slice(13,19))
            tmp = in.next
        }
    }
}

import java.net._
import java.io._
import scala.io._
import swing._
import scala.swing.event.Key
import scala.swing.event.KeyPressed
import scala.swing.BorderPanel.Position._

class UI(val out: PrintStream) extends MainFrame
{
    title = "Simple Client"

    var grille = List[Label]()
    val timer = new Label {font = new Font("Ariel", java.awt.Font.ITALIC, 50)}
    val textfield = new TextField("Input")

    for(i <- 0 to 15)
    {
        grille = new Label {font = new Font("Ariel", java.awt.Font.ITALIC, 50)}::grille
    }

    var gridPanel = new GridPanel(4,4)
    {
        for(i <- 0 to 15)
        {
            contents += grille.apply(i)
        }
    }

    contents = new BorderPanel
    {
        layout(timer) = West
        layout(gridPanel) = Center
        layout(textfield) = South
    }

    listenTo(textfield.keys)

    reactions +=
    {
        case KeyPressed(_, Key.Enter, _, _) => out.println(textfield.text)
    }

    size = new Dimension(500, 500)

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

        out.println(pseudo)

        var tmp = in.next

        while(tmp == "newPseudoRequired")
        {
            print("Le pseudo que vous avez entrer est deja pris, veuillez en choisir un autre : ")
            pseudo = scala.io.StdIn.readLine()
            out.println(pseudo)
            tmp = in.next
        }

        val ui = new UI(out)
        ui.visible = true

        while(true)
        {
            println(tmp)
            if(tmp == "newGrille") ui.updateGrille(in.next)
            if(tmp == "newTimer") ui.updateTimer(in.next)
            tmp = in.next
        }
    }
}

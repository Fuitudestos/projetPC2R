import java.net._
import java.io._
import scala.io._
import swing._

class UI extends MainFrame
{
    title = "Simple Client"

    var grille = List[Label]()
    var timer = new Label {font = new Font("Ariel", java.awt.Font.ITALIC, 50)}

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

    contents = new FlowPanel
    {
        contents += timer
        contents += gridPanel
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
        val socket = new Socket(InetAddress.getByName("localhost"), 16000)
        var in = new BufferedSource(socket.getInputStream).getLines
        val out = new PrintStream(socket.getOutputStream)

        val ui = new UI
        ui.visible = true

        var tmp = in.next

        while(true)
        {
            println(tmp)
            if(tmp == "newGrille") ui.updateGrille(in.next)
            if(tmp == "newTimer") ui.updateTimer(in.next)
            tmp = in.next
        }
    }
}

import java.net._
import java.io._
import scala.io._
import swing._

object monClient extends SimpleSwingApplication
{
    def top = new MainFrame
    {
        title = "Simple Client"

        val socket = new Socket(InetAddress.getByName("localhost"), 16000)
        var in = new BufferedSource(socket.getInputStream).getLines
        val out = new PrintStream(socket.getOutputStream)

        val gridPanel = new GridPanel(4,4)
        {
            println("Coucou")
            val grille = in.next
            for(i <- grille)
            {
                contents += new Label
                {
                    text = i+""
                    font = new Font("Ariel", java.awt.Font.ITALIC, 50)
                }
            }
        }

        contents = new FlowPanel
        {
            contents += gridPanel
        }

        size = new Dimension(500, 500)
    }
}

import java.net._
import java.io._
import scala.io._
import swing._

object monClient extends SimpleSwingApplication
{
    def top = new MainFrame
    {
        title = "Simple Client"

        val socket = new Socket(InetAddress.getByName("localhost"), 15000)
        var in = new BufferedSource(socket.getInputStream).getLines
        val out = new PrintStream(socket.getOutputStream)
        println("Client initialized:")

        contents = new BorderPanel
        {
           add(new FlowPanel
              {
                  contents += new Button(new Action("Send")
                  {
                     def apply
                     {
                       out.println("Hello!")
                       out.flush
                     }
                  })

                  contents += new Button(new Action("Close")
                  {
                     def apply
                     {
                       out.println("Disconnect")
                       out.flush
                       socket.close
                       quit
                     }
                  })
              }, BorderPanel.Position.Center)
        }

        pack
        visible = true
    }
}

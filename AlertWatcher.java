import com.rabbitmq.client.Channel;
import com.rabbitmq.client.Connection;
import com.rabbitmq.client.ConnectionFactory;
import com.rabbitmq.client.DeliverCallback;

import org.slf4j.Logger;
import org.slf4j.LoggerFactory;

import java.text.*;
import java.util.*;
import java.io.*;
import java.net.*;

public class AlertWatcher {
  private static final String EXCHANGE_NAME = "api.events";
  private static final String ROUTING_KEY = "alert.#";
  private static final DateFormat sdf = new SimpleDateFormat("yyyy/MM/dd HH:mm:ss a");
  private static String USER_AGENT = "AlertWatcher/1.0";

  private static final Logger logger = LoggerFactory.getLogger(AlertWatcher.class);
  private static HashMap<String, String> _properties = new HashMap<String, String>(16);
  private static String queueName;
  private static ConnectionFactory factory = null;
  private static Connection connection = null;
  private static Channel channel = null;
  

  public static void main(String[] argv) throws Exception {
    logger.info("Starting AlertWatcher at "+sdf.format(new Date()));
    if (argv.length == 0){
       logger.error("Need config file as argument");
       System.exit(1);
    }

   
    //LOAD CONFIG FILE
    String configFile = argv[0];
    logger.info("Attempting to load config from "+configFile);
    try {
       DataInputStream dis = new DataInputStream(new FileInputStream(new File(configFile)));
       while (dis.available() > 0){
          String line = dis.readLine();
          if (line != null && !line.isEmpty() && !line.trim().startsWith("#")){
             logger.info("Loading: "+line);
             String[] split = line.split("=");
             String key = split[0].trim();
             String value = split[1].trim();
             _properties.put(key, value);
          }
       }
    }catch(Exception e){
       logger.error("Unable to read from config file : "+e);
       e.printStackTrace();
       System.exit(1);
    }

    //GET URL
    String ARDUINO_URL = _properties.get("Alert_Arduino_URL");

    //CONNECT TO RABBITMQ
    try {
       factory = new ConnectionFactory();
       factory.setHost(_properties.get("host"));
       factory.setPort(Integer.parseInt(_properties.get("RabbitMQPort")));
       factory.setUsername("cb");
       factory.setPassword(_properties.get("RabbitMQPassword"));
       connection = factory.newConnection();
       channel = connection.createChannel();

       queueName = _properties.get("QueueName");
       channel.queueDeclare(queueName, false, false, false, null);
       channel.queueBind(queueName, EXCHANGE_NAME, ROUTING_KEY);
     }catch(Exception e){
        logger.error("Unable to connect to rabbit mq : "+e);
        e.printStackTrace();
     }

    //WAIT FOR MESSAGE 
    logger.info("Demo Execution, sysout enabled. Waiting for Alerts...");

    DeliverCallback deliverCallback = (consumerTag, delivery) -> {
        String message = new String(delivery.getBody(), "UTF-8");
        logger.debug("Received message: "+message);
        logger.info("Alert recevied, Sending Signal to Arduino...");
        String response = sendAlert(ARDUINO_URL);
        logger.info("Response from Arduino : "+response);
    };
    channel.basicConsume(queueName, true, deliverCallback, consumerTag -> { });
  }


  public static String sendAlert(String url) {
     try {
        URL obj = new URL(url);
        HttpURLConnection con = (HttpURLConnection) obj.openConnection();
	con.setRequestMethod("GET");
	con.setRequestProperty("User-Agent", USER_AGENT);
	int responseCode = con.getResponseCode();
	if (responseCode == HttpURLConnection.HTTP_OK) { // success
		BufferedReader in = new BufferedReader(new InputStreamReader(
				con.getInputStream()));
		String inputLine;
		StringBuffer response = new StringBuffer();

		while ((inputLine = in.readLine()) != null) {
			response.append(inputLine);
		}
		in.close();
                return response.toString();
	} else {
                return "error";
	} 
      }catch(Exception e){
          e.printStackTrace();
          return "error "+e.getMessage();
      }
  }
}

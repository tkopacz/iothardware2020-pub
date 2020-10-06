using CommandLine;
using System;
using Microsoft.Azure.Devices.Shared;
using Microsoft.Azure.Devices.Client;
using Microsoft.Azure.Amqp.Transport;
using System.Text;
using System.Threading.Tasks;
using System.Collections.Generic;
using System.Threading;
using Newtonsoft.Json;

namespace Net01IoTSender
{
    class Options
    {
        [Option('d',"deviceid",Default = 0,HelpText = "DeviceID (0, 1 ,...)")]
        public int DeviceId { get; set; }

        [Option('m',"delayMS",Default = 1000, HelpText = "Delay in MS - This SHOULD BE in Twins!")]
        public int delayMS { get; set; }
    }


    class Program
    {
        private static string[] connectionStrings = new []{
            "HostName=pltkdpepliot2016s1.azure-devices.net;DeviceId=net00;SharedAccessKey=..............",
            "HostName=pltkdpepliot2016s1.azure-devices.net;DeviceId=net01;SharedAccessKey=...........",
            "HostName=pltkdpepliot2016S1.azure-devices.net;DeviceId=net02;SharedAccessKey=............"
        };

        private static readonly Random s_randomGenerator = new Random();

        static volatile int delayMS;
        static DeviceClient clt = null;


        static void Main(string[] args)
        {
            var parser = new Parser(with => with.EnableDashDash = true);
            try
            {
                var result = parser.ParseArguments<Options>(args)
                    .WithParsedAsync(RunAsync);
            }
            catch (AggregateException ag)
            {
                Console.WriteLine(ag.ToString());
            }
            Console.WriteLine("Enter to finish");
            Console.ReadLine();
        }

        private static async Task desiredTwins(TwinCollection desiredProperties, object userContext)
        {
            try
            {
                Console.WriteLine("Desired property change:");
                Console.WriteLine(JsonConvert.SerializeObject(desiredProperties));

                if (desiredProperties.Contains("delayMS"))
                {
                    delayMS = desiredProperties["delayMS"];
                    Console.WriteLine("delayMS");
                    if (delayMS < 1000)
                    {
                        Console.WriteLine("delayMS < 1000");
                        //Device limitation!
                        delayMS = 1000;
                    }
                    //
                }
                TwinCollection reportedProperties = new TwinCollection();
                foreach (dynamic item in desiredProperties)
                {
                    reportedProperties[item.Key] = item.Value;
                }
                reportedProperties["delayMS"] = delayMS;
                await clt.UpdateReportedPropertiesAsync(reportedProperties);
            }
            catch (AggregateException ex)
            {
                foreach (Exception exception in ex.InnerExceptions)
                {
                    Console.WriteLine();
                    Console.WriteLine("Error when receiving desired property: {0}", exception);
                }
            }
            catch (Exception ex)
            {
                Console.WriteLine();
                Console.WriteLine("Error when receiving desired property: {0}", ex.Message);
            }
        }

        private static async Task<MethodResponse> methodDemo01(MethodRequest methodRequest, object userContext)
        {
            Console.WriteLine("methodDemo01");
            return new MethodResponse(
                UTF8Encoding.UTF8.GetBytes(JsonConvert.SerializeObject(
                    new { State = true, Msg = "OK, methodDemo01" }))
                , 0);
        }

        private static Task<MethodResponse> methodDemo02(MethodRequest methodRequest, object userContext)
        {
            Console.WriteLine("methodDemo02");
            Console.WriteLine(methodRequest.DataAsJson.ToString());
            return Task.FromResult(new MethodResponse(
                UTF8Encoding.UTF8.GetBytes(JsonConvert.SerializeObject(
                    new { State = true, Msg = "OK, methodDemo02" }))
                , 0));
        }

        private static async System.Threading.Tasks.Task RunAsync(Options arg)
        {
            clt = DeviceClient.CreateFromConnectionString(connectionStrings[arg.DeviceId], TransportType.Mqtt ); //MQTT or MQTT over WebSocket
            int count = 1;
            await clt.SetDesiredPropertyUpdateCallbackAsync(desiredTwins, null);
            await clt.SetMethodHandlerAsync("demo01", methodDemo01, null);
            await clt.SetMethodHandlerAsync("demo02", methodDemo02, null);
            
            ///Para
            delayMS = 10000;
            Twin t = await clt.GetTwinAsync();
            if (t.Properties.Reported.Contains("delayMS")) { delayMS = t.Properties.Reported["delayMS"]; }
            
            while (true)
            {
                var temperature = s_randomGenerator.Next(20, 35);
                var humidity = s_randomGenerator.Next(60, 80);

                string dataBuffer = $"{{\"deviceId\":\"net{arg.DeviceId:D2}\",\"dt\":\"{DateTime.UtcNow:yyyy-MM-ddTHH:mm.sss}\",\"messageId\":{count},\"temperature\":{temperature},\"humidity\":{humidity}}}";

                using var eventMessage = new Message(Encoding.UTF8.GetBytes(dataBuffer))
                {
                    ContentType = "application/json",
                    ContentEncoding = Encoding.UTF8.ToString(),
                };

                const int TemperatureThreshold = 30;
                bool tempAlert = temperature > TemperatureThreshold;
                eventMessage.Properties.Add("temperaturealert", tempAlert.ToString());
                Console.WriteLine($"\t{DateTime.Now}> Sending message: {count}, data: [{dataBuffer}]");
                count++;
                await clt.SendEventAsync(eventMessage);
                Thread.Sleep(delayMS);
                // if (count % 10 ==0) { delayMS = await getDelay(clt); ! }//NO NO NO - TWINS
            }
        }
    }
}

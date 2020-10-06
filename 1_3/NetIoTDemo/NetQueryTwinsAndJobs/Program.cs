using Microsoft.Azure.Devices;
using System;
using System.Text;
using System.Text.Unicode;
using System.Threading;
using System.Threading.Tasks;

namespace NetQueryTwinsAndJobs
{
    class Program
    {
        static ServiceClient _serviceClient;
        static JobClient _jobClient;
        static RegistryManager _registryManager;
        static String cnn =                                                                                                                                                                                                                                                                                                                                                                                                                                             "HostName=pltkdpepliot2016S1.azure-devices.net;SharedAccessKeyName=iothubowner;SharedAccessKey=.................";
        static async Task Main(string[] args)
        {
            
            Console.WriteLine("Service Client");
            _serviceClient = ServiceClient.CreateFromConnectionString(cnn);
            await _serviceClient.OpenAsync();
            await _serviceClient.InvokeDeviceMethodAsync("net00", new CloudToDeviceMethod("demo02"));
            await _serviceClient.CloseAsync();

            _registryManager = RegistryManager.CreateFromConnectionString(cnn);
            await _registryManager.OpenAsync();
            //_registryManager.AddDeviceAsync
            //_registryManager.GetDeviceAsync
            foreach (var item in await _registryManager.CreateQuery("select * from devices",10000).GetNextAsJsonAsync())
            {
                Console.WriteLine($"{item}");
            }
            await _registryManager.CloseAsync();


            _jobClient = JobClient.CreateFromConnectionString(cnn);
            await _jobClient.OpenAsync();
            CloudToDeviceMethod m = new CloudToDeviceMethod("demo01");
            //m.SetPayloadJson
            string jobid = Guid.NewGuid().ToString();
            await _jobClient.ScheduleDeviceMethodAsync(jobid, $"deviceId IN ['net01','net02']",m,DateTime.UtcNow, (long)TimeSpan.FromMinutes(2).TotalSeconds);
            JobResponse result;
            do
            {
                result = await _jobClient.GetJobAsync(jobid);
                Console.WriteLine("Job Status : " + result.Status.ToString());
                Thread.Sleep(2000);
            } while ((result.Status != JobStatus.Completed) &&
              (result.Status != JobStatus.Failed));


            await _jobClient.CloseAsync();
        }
    }
}

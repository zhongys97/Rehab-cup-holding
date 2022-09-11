using System;
using System.IO;
using System.Threading.Tasks;
using Microsoft.AspNetCore.Mvc;
using Microsoft.Azure.WebJobs;
using Microsoft.Azure.WebJobs.Extensions.Http;
using Microsoft.AspNetCore.Http;
using Microsoft.Extensions.Logging;
using Newtonsoft.Json;

namespace Company.Function
{

    static class CupSensorReading {
        public static double orientationX, orientationY, orientationZ;
        public static double prevMaxX, prevMaxY, prevMaxZ;
        public static double maxX, maxY, maxZ;
        public static int isVibrating = 0;

        public static double distInCM;

        public static double tolerance = Convert.ToDouble(1);

    }

    public static class saTrigger
    {


        [FunctionName("saTrigger")]
        public static async Task<IActionResult> Run(
            [HttpTrigger(AuthorizationLevel.Anonymous, "get", "post", Route = null)] HttpRequest req,
            ILogger log)
        {
            log.LogInformation("C# HTTP trigger function processed a request.");

            string requestBody = await new StreamReader(req.Body).ReadToEndAsync();
            dynamic data = JsonConvert.DeserializeObject(requestBody);

            CupSensorReading.orientationX = Convert.ToDouble(data[0].orientationX);
            CupSensorReading.orientationY = Convert.ToDouble(data[0].orientationY);
            CupSensorReading.orientationZ = Convert.ToDouble(data[0].orientationZ);
            CupSensorReading.distInCM = Convert.ToDouble(data[0].distance);

            log.LogInformation($"orientationX: {CupSensorReading.orientationX}. orientationY: {CupSensorReading.orientationY}. orientationZ: {CupSensorReading.orientationZ}. distance: {CupSensorReading.distInCM}");

            string responseMessage = $"This HTTP triggered function executed successfully.";

            return new OkObjectResult(responseMessage);
        }
    }

    public static class saTriggerWindowed
    {


        [FunctionName("saTriggerWindowed")]
        public static async Task<IActionResult> Run(
            [HttpTrigger(AuthorizationLevel.Anonymous, "get", "post", Route = null)] HttpRequest req,
            ILogger log)
        {
            log.LogInformation("C# HTTP trigger function processed a request.");

            string requestBody = await new StreamReader(req.Body).ReadToEndAsync();
            dynamic data = JsonConvert.DeserializeObject(requestBody);

            CupSensorReading.prevMaxX = CupSensorReading.maxX;
            CupSensorReading.prevMaxY = CupSensorReading.maxY;
            CupSensorReading.prevMaxZ = CupSensorReading.maxZ;


            CupSensorReading.maxX = Convert.ToDouble(data[0].maxOrientationX);
            CupSensorReading.maxY = Convert.ToDouble(data[0].maxOrientationY);
            CupSensorReading.maxZ = Convert.ToDouble(data[0].maxOrientationZ);

            if (Math.Abs(CupSensorReading.prevMaxX - CupSensorReading.maxX) > CupSensorReading.tolerance || Math.Abs(CupSensorReading.prevMaxY - CupSensorReading.maxY) > CupSensorReading.tolerance || Math.Abs(CupSensorReading.prevMaxZ - CupSensorReading.maxZ) > CupSensorReading.tolerance){
                CupSensorReading.isVibrating = 1;
            }
            else {
                CupSensorReading.isVibrating = -1;
            }



            string responseMessage = $"This HTTP triggered function executed successfully.";

            return new OkObjectResult(responseMessage);
        }
    }


    public static class unityTrigger
    {
        [FunctionName("unityTrigger")]
        public static async Task<IActionResult> Run2(
            [HttpTrigger(AuthorizationLevel.Anonymous, "get", Route = "1")] HttpRequest req,
            ILogger log
            )
        {
            log.LogInformation("C# HTTP trigger function processed a request.");

            string name = req.Query["name"];

            string requestBody = await new StreamReader(req.Body).ReadToEndAsync();
            dynamic data = JsonConvert.DeserializeObject(requestBody);
            

            string responseMessage = $"{CupSensorReading.orientationX},{CupSensorReading.orientationY},{CupSensorReading.orientationZ},{CupSensorReading.isVibrating},{CupSensorReading.distInCM}";

            return new OkObjectResult(responseMessage);
        }
    }
}

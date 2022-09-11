using System.Collections;
using System.Collections.Generic;
using System.Net.Http;
using UnityEngine;
using TMPro;
using System;

public class dispTemp : MonoBehaviour
{

    private float desiredDuration = 0.1f;
    private float elapsedTime;
    public float[] streamedData;

    public TextMeshPro display;

    public string res;

    float[] fetch_info(){
        using (var client = new HttpClient()){
            var endpoint = new Uri("https://temp-http-yishan-two.azurewebsites.net/api/1");
            var result = client.GetAsync(endpoint).Result.Content.ReadAsStringAsync().Result;
            res = result;
            string[] data = result.Split(",");
            float[] dataFloat = Array.ConvertAll(data, s => float.Parse(s));
            
            return dataFloat;
        }
        
    }

    // Start is called before the first frame update
    void Start()
    {
        streamedData = fetch_info();
        display.text = $"Temperature: {streamedData[0]}. Humidity: {streamedData[1]}%.";
    }

    // Update is called once per frame
    void Update()
    {
        elapsedTime += Time.deltaTime;

        if (elapsedTime >= desiredDuration){
            elapsedTime = 0;
            streamedData = fetch_info();
            display.text = $"Temperature: {streamedData[0]}. Humidity: {streamedData[1]}%.";
        }
    }
}

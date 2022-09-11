using System.Collections;
using System.Collections.Generic;
using System.Linq;
using System.Net.Http;
using UnityEngine;
using System;


public class updateOrientation : MonoBehaviour

{
    private Quaternion startRotation, targetRotation;

    private Vector3 startPos, endPos;
    private float desiredDuration = 0.01f;
    private float elapsedTime;
    public float[] streamedData;
    private Queue<float> distances;
    Renderer rend;

    public Color cupColor;
 
    public string res;

    // Start is called before the first frame update

    async void fetch_info(){
        using (var client = new HttpClient()){
            var endpoint = new Uri("https://rehab-cup-sa-unity-api.azurewebsites.net/api/1");
            var result = client.GetAsync(endpoint).Result.Content.ReadAsStringAsync().Result;
            res = result;
            string[] data = result.Split(",");
            streamedData = Array.ConvertAll(data, s => float.Parse(s));
            
        }
        
    }


    void Start()
    {
        rend = GetComponent<Renderer>();
        rend.enabled = true;
        distances = new Queue<float>();

        // set a 5 times MA, insert 4 0s first
        for (int i = 0; i < 4; i++){
            distances.Enqueue(0);
        }
        

        startRotation = transform.rotation;
        startPos = transform.position;
        fetch_info();
        distances.Enqueue(streamedData[4]);
        float movingAvgDist = distances.Average();
        targetRotation = Quaternion.Euler(-streamedData[1], streamedData[0], streamedData[2]);
        endPos = new Vector3(startPos.x, 3+ movingAvgDist, startPos.z);
        elapsedTime = 0;
    }

    // Update is called once per frame
    void Update()
    {
        elapsedTime += Time.deltaTime;
        float percentageCompleted = elapsedTime / desiredDuration;

        transform.rotation = Quaternion.Slerp(startRotation, targetRotation, percentageCompleted);
        transform.position = Vector3.Lerp(startPos, endPos, percentageCompleted);

        if (elapsedTime >= desiredDuration){
            elapsedTime = 0;
            startRotation = transform.rotation;
            startPos = endPos;
            fetch_info();
            distances.Dequeue();
            distances.Enqueue(streamedData[4]);
            float movingAvgDist = distances.Average();
            targetRotation = Quaternion.Euler (-streamedData[1], streamedData[0], streamedData[2]);
            endPos = new Vector3(startPos.x, 3+movingAvgDist, startPos.z);

            Debug.Log($"{transform.rotation.eulerAngles.x}, {transform.rotation.eulerAngles.y}, {transform.rotation.eulerAngles.z}");
            if (streamedData[3] > 0){
                cupColor = new Color(1.0f, 1.0f, 0f, 1f);
                rend.material.color = cupColor;
            }
            else if (streamedData[3] < 0){
                cupColor = new Color(0f, 1.0f, 0f, 1f);
                rend.material.color = cupColor;
            }
        }

    }
}

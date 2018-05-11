using System.Collections.Generic;
using UnityEngine;

public class SOM {
	
	public float learningRate;

	float variance;

	int nInputs;

	int width;
	int height;

	List<float>[] centroids;

	public SOM(int nInputs, int width, int height, float learningRate, float variance=1.0f) {
		this.nInputs = nInputs;
		this.width = width;
		this.height = height;
		this.learningRate = learningRate;
		this.variance = variance;
		centroids = new List<float>[width*height];
	}

	public void setup() {
		// Randomly initialize centroids.
		for (int i = 0; i < nClusters(); i++) {
			List<float> centroid = new List<float> ();
			for (int j = 0; j < nInputs; j++)
				centroid.Add(Random.Range (-1.0f, +1.0f));
			centroids [i] = centroid;
		}
	}

	public int nClusters() {
		return centroids.Length;
	}

	public int getX(int cluster) {
		return cluster % width;
	}

	public int getY(int cluster) {
		return cluster / width;
	}

	public int step(List<float> input) {
		int cluster = getCluster(input);
		for (int c=0; c<nClusters(); c++) {
			List<float> centroid = centroids [c];
			float lr = learningRate * neighborhood(cluster, c);
			for (int i=0; i<nInputs; i++) {
				centroid[i] += (input[i] - centroid[i]) * lr;
			}
		}
		return cluster;
	}


	float neighborhood(int cluster, int neighbor) {
		if (cluster == neighbor)
			return 1;
		int xCluster  = getX(cluster);
		int yCluster  = getY(cluster);
		int xNeighbor = getX(neighbor);
		int yNeighbor = getY(neighbor);
		float distX = (xCluster - xNeighbor);
		float distY = (yCluster - yNeighbor);
		return Mathf.Exp( - (distX*distX + distY*distY ) / (2*variance + 1.175494351e-38f ) ); // 1.175494351e-38f is FLT_MIN
	}

	public int getCluster(List<float> x) {
		float minDist2 = 1e+10f;
		int closest = -1;
		for (int i = 0; i < nClusters(); i++) {
			float dist2 = distSq (x, centroids[i]);
			if (dist2 < minDist2) {
				minDist2 = dist2;
				closest = i;
			}
		}

		return closest;
	}


	float distSq(List<float> x1, List<float> x2) {
		float dist2 = 0;
		for (int i = 0; i < x1.Count; i++) {
			float diff = x1[i] - x2[i];
			dist2 += diff*diff;
		}
		return dist2;
	}
};

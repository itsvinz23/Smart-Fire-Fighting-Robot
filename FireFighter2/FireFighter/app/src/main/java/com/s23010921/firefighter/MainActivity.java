package com.s23010921.firefighter;

import android.os.Bundle;
import android.widget.TextView;
import android.widget.Toast;

import androidx.appcompat.app.AppCompatActivity;

import com.google.firebase.analytics.FirebaseAnalytics;
import com.google.firebase.database.DataSnapshot;
import com.google.firebase.database.DatabaseError;
import com.google.firebase.database.DatabaseReference;
import com.google.firebase.database.FirebaseDatabase;
import com.google.firebase.database.ValueEventListener;

public class MainActivity extends AppCompatActivity {
    private TextView temperatureValue, humidityValue, waterTankValue, fireDistanceValue, flameDetectedValue;

    @Override
    protected void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);
        setContentView(R.layout.activity_main);

        // Firebase Analytics (just to confirm connection)
        FirebaseAnalytics mFirebaseAnalytics = FirebaseAnalytics.getInstance(this);
        Bundle bundle = new Bundle();
        bundle.putString(FirebaseAnalytics.Param.METHOD, "check_connection");
        mFirebaseAnalytics.logEvent(FirebaseAnalytics.Event.LOGIN, bundle);

        // Link UI elements
        temperatureValue = findViewById(R.id.temperatureValue);
        humidityValue = findViewById(R.id.humidityValue);
        waterTankValue = findViewById(R.id.waterTankValue);
        fireDistanceValue = findViewById(R.id.fireDistanceValue);
        flameDetectedValue = findViewById(R.id.flameDetectedValue);

        // Firebase Realtime Database reference
        DatabaseReference sensorRef = FirebaseDatabase.getInstance().getReference("sensors");

        // Listener for real-time updates
        sensorRef.addValueEventListener(new ValueEventListener() {
            @Override
            public void onDataChange(DataSnapshot snapshot) {
                // Read and set values
                Double temp = snapshot.child("temperature").getValue(Double.class);
                Integer humidity = snapshot.child("humidity").getValue(Integer.class);
                String water = snapshot.child("waterTank").getValue(String.class);
                Double fireDist = snapshot.child("fireDistance").getValue(Double.class);
                String flame = snapshot.child("flameDetected").getValue(String.class);

                // Show Toast for each value
                if (temp != null) {
                    temperatureValue.setText(temp + " °C");
                    Toast.makeText(MainActivity.this, "Temperature: " + temp, Toast.LENGTH_SHORT).show();
                } else {
                    Toast.makeText(MainActivity.this, "Temperature data missing", Toast.LENGTH_SHORT).show();
                }

                if (humidity != null) {
                    humidityValue.setText(humidity + " %");
                    Toast.makeText(MainActivity.this, "Humidity: " + humidity, Toast.LENGTH_SHORT).show();
                } else {
                    Toast.makeText(MainActivity.this, "Humidity data missing", Toast.LENGTH_SHORT).show();
                }

                if (water != null) {
                    waterTankValue.setText(water);
                    Toast.makeText(MainActivity.this, "Water Tank: " + water, Toast.LENGTH_SHORT).show();
                } else {
                    Toast.makeText(MainActivity.this, "Water Tank data missing", Toast.LENGTH_SHORT).show();
                }

                if (fireDist != null) {
                    fireDistanceValue.setText(fireDist + " m");
                    Toast.makeText(MainActivity.this, "Fire Distance: " + fireDist, Toast.LENGTH_SHORT).show();
                } else {
                    Toast.makeText(MainActivity.this, "Fire Distance data missing", Toast.LENGTH_SHORT).show();
                }

                if (flame != null) {
                    flameDetectedValue.setText(flame);
                    Toast.makeText(MainActivity.this, "Flame Detected: " + flame, Toast.LENGTH_SHORT).show();
                } else {
                    Toast.makeText(MainActivity.this, "Flame Detected data missing", Toast.LENGTH_SHORT).show();
                }
            }

            @Override
            public void onCancelled(DatabaseError error) {
                Toast.makeText(MainActivity.this, "Firebase error: " + error.getMessage(), Toast.LENGTH_LONG).show();
            }
        });
    }
}

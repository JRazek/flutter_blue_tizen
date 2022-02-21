import 'dart:async';
import 'dart:io';
import 'dart:math';

import 'package:flutter/material.dart';
import 'package:flutter_blue/flutter_blue.dart';

List<int> _getRandomBytes() {
  final math = Random();
  return [
    math.nextInt(255),
    math.nextInt(255),
    math.nextInt(255),
    math.nextInt(255)
  ];
}

void main() async {
  WidgetsFlutterBinding.ensureInitialized();
  FlutterBlue flutterBlue = FlutterBlue.instance;
  await flutterBlue.stopScan();
  List<ScanResult> scanResults = <ScanResult>[];
  var subscription = flutterBlue.scanResults.listen((results) async {
    if (results.isNotEmpty) {
      var dev = results.last.device;
      var sc = results.last;
      scanResults.add(results.last);
    }
  });
  await flutterBlue.startScan(timeout: const Duration(seconds: 3));
  debugPrint("scanned");
  for (var sc in scanResults) {
    var dev = sc.device;
    if (sc.advertisementData.localName == "Galaxy S20 FE JRazek") {
      await dev.connect(autoConnect: false);
      var services = await dev.discoverServices();
      for (var service in services) {
        for (var characteristic in service.characteristics) {
          if (characteristic.properties.read &&
              characteristic.properties.write) {
            List<int> values = await characteristic.read();
            try {
              await characteristic
                  .write([65, 66, 67, 68, 70], withoutResponse: true);
              // debugPrint(values.toString());
            } catch (e) {
              debugPrint(service.uuid.toString() +
                  " " +
                  characteristic.uuid.toString());
            }
            values = await characteristic.read();
            debugPrint(values.toString());
          }
          if ((characteristic.properties.notify ||
              characteristic.properties.indicate)) {
            characteristic.setNotifyValue(true);
            // var charNotify=characteristic.
          }
        }
      }
      await dev.requestMtu(321);
      var mtuSub = dev.mtu.listen((mtu) async {
        debugPrint(mtu.toString());
      });
      debugPrint("released");
      await dev.disconnect();
    }
  }
  runApp(const MyApp());
}

class MyApp extends StatefulWidget {
  const MyApp({Key? key}) : super(key: key);

  @override
  State<MyApp> createState() => _MyAppState();
}

class _MyAppState extends State<MyApp> {
  @override
  void initState() {
    super.initState();
  }

  @override
  Widget build(BuildContext context) {
    return MaterialApp(
      home: Scaffold(
        appBar: AppBar(
          title: const Text('Plugin example app'),
        ),
        body: const Center(
          child: Text('Running on: '),
        ),
      ),
    );
  }
}

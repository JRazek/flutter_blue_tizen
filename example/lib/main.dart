import 'dart:async';
import 'dart:io';

import 'package:flutter/material.dart';
import 'package:flutter_blue/flutter_blue.dart';

class BluetoothHelper {}

void main() async {
  WidgetsFlutterBinding.ensureInitialized();
  FlutterBlue flutterBlue = FlutterBlue.instance;
  // flutterBlue.setLogLevel(LogLevel.alert);
  await flutterBlue.stopScan();
  List<ScanResult> scanResults = <ScanResult>[];
  var subscription = flutterBlue.scanResults.listen((results) async {
    // do something with scan results
    if (results.isNotEmpty) {
      var dev = results.last.device;
      var sc = results.last;
      debugPrint(dev.toString());
      scanResults.add(results.last);
    }
  });
  await flutterBlue.startScan(timeout: const Duration(seconds: 5));

  for (var sc in scanResults) {
    var dev = sc.device;
    if (sc.advertisementData.localName == "Galaxy S20 FE JRazek") {
      debugPrint('connecting to: ${sc.advertisementData.localName}');
      // await dev.disconnect();
      await dev.connect(autoConnect: false);
      // sleep(const Duration(seconds: 28));
      var services = await dev.discoverServices();
      for (var service in services) {
        debugPrint(service.toString());
      }
      dev.disconnect();
      debugPrint('released connect');
    }
  }

  // Stop scanning
  debugPrint("hello");
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

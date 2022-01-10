import 'dart:async';

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
      debugPrint('${dev.name}, local_name: ${sc.advertisementData.localName}');
      // if (sc.advertisementData.localName == "Galaxy S20 FE JRazek") {
      //   debugPrint("herreeeeee");
      // }
      dev.connect();
      scanResults.add(results.last);
    }
  });
  flutterBlue.startScan();

  for (var sc in scanResults) {
    var dev = sc.device;
    debugPrint('${dev.name}, local_name: ${sc.advertisementData.localName}');
    if (sc.advertisementData.localName == "Galaxy S20 FE JRazek") dev.connect();
    // dev.connect();
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

import 'package:flutter/material.dart';
import 'package:flutter_blue/flutter_blue.dart';

void main() async {
  WidgetsFlutterBinding.ensureInitialized();
  FlutterBlue flutterBlue = FlutterBlue.instance;
  // flutterBlue.
  // await flutterBlue.stopScan();
  flutterBlue.setLogLevel(LogLevel.error);
  flutterBlue.startScan(timeout: const Duration(seconds: 3));
  // flutterBlue.scanResults;
  // flutterBlue.scanResults.listen((data) {
  //   debugPrint("devices found - " + data.length.toString());
  // });
  // List<BluetoothDevice> l = await flutterBlue.connectedDevices;

  // debugPrint("Here1232123212312312312312312");
  // debugPrint((await flutterBlue.isAvailable).toString());
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

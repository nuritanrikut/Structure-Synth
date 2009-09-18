#if defined(_MSC_VER) 
    // disable warning "'QtConcurrent::BlockSizeManager' : assignment operator could not be generated"
    #pragma warning( disable : 4512 )
#endif

#include "JavaScriptParser.h"

#include <QScriptEngine>
#include <QMetaType>
#include "SyntopiaCore/Logging/Logging.h"
#include "SyntopiaCore/GLEngine/Sphere.h"

#include "Debug.h"

using namespace SyntopiaCore::Logging;

//Q_DECLARE_METATYPE(StructureSynth::JavaScriptSupport::Vector3)


namespace StructureSynth {
	namespace JavaScriptSupport {	

		namespace {
			QScriptValue constructVector3(QScriptContext * context, QScriptEngine *engine)
			{
				//INFO("constructVector3");
				StructureSynth::JavaScriptSupport::Vector3* bar = new StructureSynth::JavaScriptSupport::Vector3();
				// initialize from arguments in context, if desired
				//QScriptValue v =  engine->toScriptValue(bar);
				QScriptValue v =  engine->newQObject(bar, QScriptEngine::ScriptOwnership);
				//INFO("Old X: " + v.property("x").toString());
				if (context->argumentCount() == 3) {
					v.setProperty("x", QScriptValue(engine,context->argument(0).toNumber()));
					v.setProperty("y", QScriptValue(engine,context->argument(1).toNumber()));
					v.setProperty("z", QScriptValue(engine,context->argument(2).toNumber()));
				}
				//INFO("constructVector3 - end");
				
				return v;
			}

			QScriptValue vector3ToScriptValue(QScriptEngine *engine, const StructureSynth::JavaScriptSupport::Vector3 &s)
			{
				//INFO("<-toScriptValue: From " + s.toString());
				QScriptValue obj = engine->newObject();
				/*
				Vector3 v;
				v.writeX(12);
				v.writeY(22);
				v.writeZ(32);
				obj.setPrototype(engine->newQObject(&v));
				*/
			
				obj.setProperty("x", QScriptValue(engine, s.readX()));
				obj.setProperty("y", QScriptValue(engine, s.readY()));
				obj.setProperty("z", QScriptValue(engine, s.readZ()));
				return obj;
			}

			void vector3FromScriptValue(const QScriptValue &obj, StructureSynth::JavaScriptSupport::Vector3 &s)
			{
				//INFO("<-fromScriptValue");
				s.writeX( obj.property("x").toNumber() );
				s.writeY( obj.property("y").toNumber() );
				s.writeZ( obj.property("z").toNumber() );
			} 

			QScriptValue addSphereStatic(QScriptContext* context, QScriptEngine* /*engine*/)
			{
				QScriptValue thisObject = context->thisObject();
				QObject* w = thisObject.toQObject();
				
				if (context->argumentCount() == 2) {
					Vector3 v = qScriptValueToValue<Vector3>(context->argument(0));
					double radius = context->argument(1).toNumber();
					//INFO("Got: " + v.toString() + " and " + QString::number(radius));
					SyntopiaCore::GLEngine::Object3D* o = new SyntopiaCore::GLEngine::Sphere( v.getObj(), radius);
					o->setColor(((World*)w)->getRgb(), ((World*)w)->getAlpha());
					((World*)w)->getEngine()->addObject(o);
				} else {
					WARNING("addSphere expected two arguments..");
				}
				return QScriptValue();
			}
				
			QScriptValue setColorStatic(QScriptContext* context, QScriptEngine* /*engine*/)
			{
				QScriptValue thisObject = context->thisObject();
				QObject* w = thisObject.toQObject();
				
				if (context->argumentCount() == 2) {
					Vector3 v = qScriptValueToValue<Vector3>(context->argument(0));
					double alpha = context->argument(1).toNumber();
					((World*)w)->setColor2(v, alpha);
				} else {
					WARNING("setColor expected two arguments..");
				}
				return QScriptValue();
			}
			

		}

		JavaScriptParser::JavaScriptParser(SyntopiaCore::GLEngine::EngineWidget* engine3D) : engine3D(engine3D) {
		}

		JavaScriptParser::~JavaScriptParser() {
		}
			
		void JavaScriptParser::parse(QString input) {

			INFO("Initializing JavaScript environment.");
			QScriptEngine engine;

			// Setup the global objects...
			Debug debugObject;
			engine.globalObject().setProperty("Debug", engine.newQObject(&debugObject)); 

			Builder builder(engine3D);
			engine.globalObject().setProperty("Builder", engine.newQObject(&builder)); 



			World world(engine3D);
			QScriptValue w = engine.newQObject(&world);
			w.setProperty("addSphere", engine.newFunction(addSphereStatic));
			w.setProperty("setColor", engine.newFunction(setColorStatic));
			engine.globalObject().setProperty("World", w); 
			 

			Vector3 *v3PrototypeObject = new Vector3();
			QScriptValue vProto = engine.newQObject(v3PrototypeObject);
			int id = qMetaTypeId<StructureSynth::JavaScriptSupport::Vector3>();
			engine.setDefaultPrototype(id, vProto);
			QScriptValue vectorCtor = engine.newFunction(constructVector3, vProto);
			engine.globalObject().setProperty("Vector3", vectorCtor); 

			qScriptRegisterMetaType(&engine, vector3ToScriptValue, vector3FromScriptValue, vProto); 
			//qScriptRegisterMetaType(&engine, vector3ToScriptValue, vector3FromScriptValue); 

			// Execute and catch exceptions.
			QScriptValue result = engine.evaluate(input);
			if (engine.hasUncaughtException()) {
				int line = engine.uncaughtExceptionLineNumber();
				QString error =  QString("Uncaught exception at line %1:%2").arg(line).arg(result.toString());
				WARNING(error);
			} else {
				INFO(result.toString());
			}

		}
		
	}
}
